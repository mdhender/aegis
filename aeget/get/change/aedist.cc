//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <libaegis/change.h>
#include <aeget/get/change/aedist.h>
#include <aeget/get/command.h>
#include <common/libdir.h>
#include <libaegis/project.h>
#include <common/str_list.h>


void
get_change_aedist(change::pointer cp, string_ty *, string_list_ty *modifier)
{
    string_ty       *qp;
    string_ty       *command;
    int             entire_source = 0;
    string_ty       *compatibility = 0;
    size_t          j;

    //
    // See of they specified any options.
    //
    for (j = 0; j < modifier->nstrings; ++j)
    {
	const char      *s;

	s = modifier->string[j]->str_text;
	if (0 == strcasecmp(s, "es"))
	    entire_source = 1;
	if (0 == strcasecmp(s, "nopatch"))
	    compatibility = str_from_c(" -compat=4.6");
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
    // The "-naa" option (same as "-cte=none") means don't bother using
    // base64 encoding around it.  Straight binary will download faster.
    //
    qp = str_quote_shell(project_name_get(cp->pp));
    command =
	str_format
	(
	    "%s/aedist -send --project=%s -change=%ld --mime-header "
		"--content-transfer-encoding=none -compress -ndh %s %s",
	    configured_bindir(),
	    qp->str_text,
	    magic_zero_decode(cp->number),
	    (entire_source ? "--entire-source" : ""),
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
