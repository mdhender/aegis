/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001-2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate lockss
 */

#include <ac/pwd.h>

#include <ael/change/inappropriat.h>
#include <ael/project/inappropriat.h>
#include <ael/locks.h>
#include <col.h>
#include <gonzo.h>
#include <itab.h>
#include <lock.h>
#include <output.h>
#include <project.h>
#include <str.h>
#include <str_list.h>
#include <trace.h>
#include <zero.h>


static col_ty	*colp;
static output_ty *list_locks_name_col;
static output_ty *list_locks_type_col;
static output_ty *list_locks_project_col;
static output_ty *list_locks_change_col;
static output_ty *list_locks_address_col;
static output_ty *list_locks_process_col;
static string_list_ty list_locks_pnames;
static long	list_locks_count;


static void
list_locks_callback(lock_walk_found *found)
{
    const char      *name_str;
    const char      *type_str;
    const char      *project_str;
    long	    change_number;
    long	    j;
    static itab_ty  *user_name_by_uid;
    string_ty	    *s;

    list_locks_count++;
    name_str = "unknown";
    switch (found->name)
    {
    case lock_walk_name_master:
	name_str = "master";
	break;

    case lock_walk_name_gstate:
	name_str = "gonzo";
	break;

    case lock_walk_name_pstate:
	name_str = "project";
	break;

    case lock_walk_name_cstate:
	name_str = "change";
	break;

    case lock_walk_name_ustate:
	name_str = "user";
	break;

    case lock_walk_name_baseline:
	name_str = "baseline";
	break;

    case lock_walk_name_baseline_priority:
	name_str = "baseline priority";
	break;

    case lock_walk_name_history:
	name_str = "history";
	break;

    case lock_walk_name_unknown:
	break;
    }

    type_str = "unknown";
    switch (found->type)
    {
    case lock_walk_type_shared:
	type_str = "shared";
	break;

    case lock_walk_type_exclusive:
	type_str = "exclusive";
	break;

    case lock_walk_type_unknown:
	break;
    }

    project_str = 0;
    change_number = 0;
    switch (found->name)
    {
    case lock_walk_name_pstate:
    case lock_walk_name_baseline:
    case lock_walk_name_baseline_priority:
    case lock_walk_name_history:
	for (j = 0; j < list_locks_pnames.nstrings; ++j)
	{
	    s = list_locks_pnames.string[j];
	    if ((s->str_hash & 0xFFFF) == found->subset)
	    {
		project_str = s->str_text;
		break;
	    }
	}
	if (!project_str)
	    project_str = "unknown";
	break;

    case lock_walk_name_cstate:
	for (j = 0; j < list_locks_pnames.nstrings; ++j)
	{
	    long	    cn;

	    /*
	     * This is very messy, because the change
	     * number is added to the project name hash.
	     *
	     * Work out the change number,
	     * and the assume it can't be more than,
	     * say, 5000.  We could do better by looking
	     * in each project, but what the hell.
	     *
	     * Use (mod 2**16) arithmetic, that's how its done.
	     * Should bring this out in lock.h if ever change.
	     *
	     * Loop and find the smallest change number.
	     * Use that as a probable "best" fit.
	     */
	    s = list_locks_pnames.string[j];
	    cn = (found->subset - s->str_hash) & 0xFFFF;
	    if (cn >= 1 && cn <= 5000 && (!change_number || cn < change_number))
	    {
		project_str = s->str_text;
		change_number = cn;
	    }
	}
	if (!project_str)
	    project_str = "unknown";
	break;

    case lock_walk_name_ustate:
	/* This is the UID */
	change_number = found->subset;

	if (!user_name_by_uid)
	{
	    user_name_by_uid = itab_alloc(100);
	    setpwent();
	    for (;;)
	    {
		struct passwd	*pw;

		pw = getpwent();
		if (!pw)
		    break;
		s = str_from_c(pw->pw_name);
		itab_assign(user_name_by_uid, pw->pw_uid, s);
	    }
	    endpwent();
	}
	s = itab_query(user_name_by_uid, change_number);
	project_str = s ? s->str_text : "unknown";
	break;

    case lock_walk_name_master:
    case lock_walk_name_gstate:
    case lock_walk_name_unknown:
	break;
    }

    /*
     * print it all out
     */
    output_fputs(list_locks_name_col, name_str);
    output_fputs(list_locks_type_col, type_str);
    if (project_str)
	output_fputs(list_locks_project_col, project_str);
    if (change_number)
    {
	output_fprintf
	(
	    list_locks_change_col,
	    "%4ld",
	    magic_zero_decode(change_number)
	);
    }
    output_fprintf(list_locks_address_col, "%8.8lX", found->address);
    output_fprintf(list_locks_process_col, "%5d", found->pid);
    if (!found->pid_is_local)
	output_fputs(list_locks_process_col, " remote");
    col_eoln(colp);
}


void
list_locks(string_ty *project_name, long change_number, string_list_ty *args)
{
    /*
     * check for silly arguments
     */
    trace(("list_locks()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();

    /*
     * get the list of projects
     */
    project_list_get(&list_locks_pnames);

    /*
     * open the columns
     */
    colp = col_open((string_ty *)0);
    col_title(colp, "List of Locks", gonzo_lockpath_get()->str_text);
    list_locks_name_col = col_create(colp, 0, 8, "Type\n------");
    list_locks_type_col = col_create(colp, 9, 19, "Mode\n------");
    list_locks_project_col = col_create(colp, 20, 32, "Project\n---------");
    list_locks_change_col = col_create(colp, 33, 40, "Change\n------");
    list_locks_address_col = col_create(colp, 41, 50, "Address\n--------");
    list_locks_process_col = col_create(colp, 51, 0, "Process\n--------");
    list_locks_count = 0;

    /*
     * list the locks found
     */
    lock_walk(list_locks_callback);
    string_list_destructor(&list_locks_pnames);
    if (list_locks_count == 0)
    {
	output_ty	*info;

	info = col_create(colp, 4, 0, (const char *)0);
	output_fputs(info, "No locks found.");
	col_eoln(colp);
    }
    col_close(colp);

    /*
     * clean up
     */
    colp = 0;
    list_locks_name_col = 0;
    list_locks_type_col = 0;
    list_locks_project_col = 0;
    list_locks_change_col = 0;
    list_locks_address_col = 0;
    list_locks_process_col = 0;
    list_locks_count = 0;
    trace(("}\n"));
}
