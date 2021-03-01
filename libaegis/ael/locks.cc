//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2009, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/pwd.h>

#include <common/itab.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/locks.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/gonzo.h>
#include <libaegis/lock.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/zero.h>


static col::pointer colp;
static output::pointer list_locks_name_col;
static output::pointer list_locks_type_col;
static output::pointer list_locks_project_col;
static output::pointer list_locks_change_col;
static output::pointer list_locks_address_col;
static output::pointer list_locks_process_col;
static string_list_ty list_locks_pnames;
static long     list_locks_count;


static void
list_locks_callback(lock_walk_found *found)
{
    const char      *name_str;
    const char      *type_str;
    const char      *project_str;
    long            change_number;
    size_t          j;
    static itab_ty  *user_name_by_uid;
    string_ty       *s;

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
    trace(("looking for found->subset 0x%04x\n", (unsigned)found->subset));
    switch (found->name)
    {
    case lock_walk_name_pstate:
    case lock_walk_name_baseline:
    case lock_walk_name_baseline_priority:
    case lock_walk_name_history:
        for (j = 0; j < list_locks_pnames.nstrings; ++j)
        {
            s = list_locks_pnames.string[j];
            trace(("project %s (0x%04x)\n", s->str_text,
                (unsigned)s->str_hash));
            if ((s->str_hash & 0xFFFF) == (unsigned)found->subset)
            {
                project_str = s->str_text;
                break;
            }
        }
        if (!project_str)
            project_str = "unknown";
        break;

    case lock_walk_name_cstate:
        for (j = 0; j < list_locks_pnames.nstrings && !change_number; ++j)
        {
            s = list_locks_pnames.string[j];
            project *pp = project_alloc(s);
            if (!pp->bind_existing_errok())
            {
                project_free(pp);
                continue;
            }

            //
            // This is very messy, because the change
            // number is added to the project name hash.
            //
            // Use (mod 2**16) arithmetic, that's how its done.
            // Should bring this out in lock.h if ever change.
            //
            long cn = (found->subset - s->str_hash) & 0xFFFF;
            trace(("project %s (0x%04x), change %ld (0x%04x)\n", s->str_text,
                (unsigned)s->str_hash, cn, (unsigned)cn));
            change::pointer cp = change_alloc(pp, cn);
            bool exists = change_bind_existing_errok(cp);
            change_free(cp);
            if (exists)
            {
                // A change with this change number exists in the
                // project.  Note that there could be more than one
                // {project, change} combination that produces this
                // lock number, and this code reports only the first
                // one found.
                project_str = s->str_text;
                change_number = magic_zero_encode(cn);
            }
            project_free(pp);
        }
        if (!project_str)
            project_str = "unknown";
        break;

    case lock_walk_name_ustate:
        // This is the UID
        change_number = found->subset;

        if (!user_name_by_uid)
        {
            user_name_by_uid = itab_alloc();
            setpwent();
            for (;;)
            {
                struct passwd   *pw;

                pw = getpwent();
                if (!pw)
                    break;
                s = str_from_c(pw->pw_name);
                itab_assign(user_name_by_uid, pw->pw_uid, s);
            }
            endpwent();
        }
        s = (string_ty *)itab_query(user_name_by_uid, change_number);
        project_str = s ? s->str_text : "unknown";
        break;

    case lock_walk_name_master:
    case lock_walk_name_gstate:
    case lock_walk_name_unknown:
        break;
    }

    //
    // print it all out
    //
    list_locks_name_col->fputs(name_str);
    list_locks_type_col->fputs(type_str);
    if (project_str)
        list_locks_project_col->fputs(project_str);
    if (change_number)
    {
        list_locks_change_col->fprintf
        (
            "%4ld",
            magic_zero_decode(change_number)
        );
    }
    list_locks_address_col->fprintf("%8.8lX", found->address);
    list_locks_process_col->fprintf("%5d", found->pid);
    if (!found->pid_is_local)
        list_locks_process_col->fputs(" remote");
    colp->eoln();
}


void
list_locks(change_identifier &cid, string_list_ty *)
{
    //
    // check for silly arguments
    //
    trace(("list_locks()\n{\n"));
    if (cid.project_set())
        list_project_inappropriate();
    if (cid.set())
        list_change_inappropriate();

    //
    // get the list of projects
    //
    project_list_get(&list_locks_pnames);

    //
    // open the columns
    //
    colp = col::open((string_ty *)0);
    colp->title("List of Locks", gonzo_lockpath_get()->str_text);
    list_locks_name_col = colp->create(0, 8, "Type\n------");
    list_locks_type_col = colp->create(9, 19, "Mode\n------");
    list_locks_project_col = colp->create(20, 32, "Project\n---------");
    list_locks_change_col = colp->create(33, 40, "Change\n------");
    list_locks_address_col = colp->create(41, 50, "Address\n--------");
    list_locks_process_col = colp->create(51, 0, "Process\n--------");
    list_locks_count = 0;

    //
    // list the locks found
    //
    lock_walk(list_locks_callback);
    if (list_locks_count == 0)
    {
        output::pointer info = colp->create(4, 0, (const char *)0);
        info->fputs("No locks found.");
        colp->eoln();
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
