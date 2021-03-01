/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate gets
 */

#include <change.h>
#include <error.h> /* for assert */
#include <os.h>
#include <sub.h>
#include <trace.h>


string_ty *
change_logfile_get(change_ty *cp)
{
    string_ty	*s1;
    cstate_ty       *cstate_data;

    trace(("change_logfile_get(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    if (!cp->logfile)
    {
	cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	default:
	    change_fatal(cp, 0, i18n("no log file"));

	case cstate_state_being_integrated:
	    s1 = change_integration_directory_get(cp, 0);
	    break;

	case cstate_state_being_developed:
	    s1 = change_development_directory_get(cp, 0);
	    break;
	}

	cp->logfile = os_path_cat(s1, change_logfile_basename());
    }
    trace(("return \"%s\";\n", cp->logfile->str_text));
    trace(("}\n"));
    return cp->logfile;
}
