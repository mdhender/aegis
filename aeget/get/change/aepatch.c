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
 * MANIFEST: functions to manipulate aepatchs
 */

#include <ac/string.h>

#include <change.h>
#include <get/change/aepatch.h>
#include <get/command.h>
#include <libdir.h>
#include <project.h>
#include <str_list.h>


void
get_change_aepatch(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    string_ty       *qp;
    string_ty       *command;

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
	    "%s/aepatch -send -p %s -c %ld -naa\n",
	    configured_bindir(),
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
