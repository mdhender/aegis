/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate files
 */

#include <ac/string.h>

#include <change.h>
#include <get/file.h>
#include <get/file/activity.h>
#include <get/file/contents.h>
#include <get/file/history.h>
#include <get/file/menu.h>
#include <get/file/metrics.h>
#include <str_list.h>


typedef struct table_ty table_ty;
struct table_ty
{
    const char      *name;
    void            (*action)(change_ty *, string_ty *, string_list_ty *);
};


static const table_ty table[] =
{
    { "activity", get_file_activity },
    { "conflict", get_file_activity },
    { "contents", get_file_contents },
    { "history", get_file_history },
    { "menu", get_file_menu },
    { "metrics", get_file_metrics },
};


void
get_file(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    if (modifier->nstrings >= 1)
    {
	const table_ty  *tp;
	string_ty       *name;

	/*
	 * To remove ambiguity at the root level, sometimes you have
	 * to say "@@file@history" to distinguish the request from
	 * "@@project@history"
	 */
	name = modifier->string[0];
	if
	(
	    0 != strcasecmp(name->str_text, "project")
	&&
	    0 != strcasecmp(name->str_text, "change")
	)
	{
	    if
	    (
		modifier->nstrings >= 2
	    &&
		0 == strcasecmp(name->str_text, "file")
	    )
		name = modifier->string[1];
	    for (tp = table; tp < ENDOF(table); ++tp)
	    {
		if (0 == strcasecmp(tp->name, name->str_text))
		{
		    tp->action(cp, filename, modifier);
		    return;
		}
	    }
	}
    }
    get_file_contents(cp, filename, modifier);
}
