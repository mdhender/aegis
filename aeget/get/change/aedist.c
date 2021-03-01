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
 * MANIFEST: functions to manipulate aedists
 */

#include <ac/string.h>

#include <change.h>
#include <get/change/aedist.h>
#include <get/command.h>
#include <libdir.h>
#include <project.h>
#include <str_list.h>


void
get_change_aedist(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    string_ty       *qp;
    string_ty       *command;
    int             entire_source = 0;
    int             no_patch = 0;
    size_t          j;

    /*
     * see of the specified any options.
     */
    for (j = 1; j < modifier->nstrings; ++j)
    {
	const char      *s;

	s = modifier->string[j]->str_text;
	if (0 == strcasecmp(s, "es"))
	    entire_source = 1;
	if (0 == strcasecmp(s, "nopatch"))
	    no_patch = 1;
    }

    /*
     * Build the command to be executed.
     *
     * The "-naa" option (same as "-cte=none") menas don't bother using
     * base64 encoding around it.  Straight binary will download faster.
     */
    qp = str_quote_shell(project_name_get(cp->pp));
    command =
	str_format
	(
	    "%s/aedist -send %s %s -p %s -c %ld -naa -ndh\n",
	    configured_bindir(),
	    (entire_source ? "-es" : ""),
	    (no_patch ? "-nopatch" : ""),
	    qp->str_text,
	    cp->number
	);
    str_free(qp);

    /*
     * Run the command.
     * This will exec() and does not return.
     */
    get_command(command->str_text);
    /* NOTREACHED */
}
