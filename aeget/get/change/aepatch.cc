//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate aepatchs
//

#include <common/ac/string.h>

#include <libaegis/change.h>
#include <aeget/get/change/aepatch.h>
#include <aeget/get/command.h>
#include <common/libdir.h>
#include <libaegis/project.h>
#include <common/str_list.h>


void
get_change_aepatch(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    string_ty       *qp;
    string_ty       *command;
    string_ty       *compatibility = 0;
    size_t          j;

    //
    // See of they specified any options.
    //
    for (j = 0; j < modifier->nstrings; ++j)
    {
	const char      *s;

	s = modifier->string[j]->str_text;
	if (0 == memcmp(s, "compat=", 7))
	{
	    string_ty       *uqs;
	    string_ty       *qs;

	    //
	    // We need to quote the argument in case Bad People
	    // put semicolons and other naughty things in it.
	    //
	    uqs = str_from_c(s + 7);
	    qs = str_quote_shell(uqs);
	    str_free(uqs);
	    compatibility = str_format(" -compat=%s", qs->str_text);
	    str_free(qs);
	}
    }

    //
    // Build the command to be executed.
    //
    // The "-naa" option (same as "-cte=none") menas don't bother using
    // base64 encoding around it.  Straight binary will download faster.
    //
    qp = str_quote_shell(project_name_get(cp->pp));
    command =
	str_format
	(
	    "%s/aepatch -send -p %s -c %ld -naa%s",
	    configured_bindir(),
	    qp->str_text,
	    magic_zero_decode(cp->number),
	    (compatibility ? compatibility->str_text : "")
	);
    str_free(qp);

    //
    // Run the command.
    // This will exec() and does not return.
    //
    get_command(command->str_text);
    // NOTREACHED
}
