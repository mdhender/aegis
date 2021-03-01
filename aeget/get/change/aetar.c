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
 * MANIFEST: functions to manipulate aetars
 */

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/string.h>

#include <change.h>
#include <cstate.h>
#include <get/change/aetar.h>
#include <get/command.h>
#include <libdir.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>


static string_ty *
change_brief_description_get(change_ty *cp)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    return cstate_data->brief_description;
}


static int
len_printable(string_ty *s, int max)
{
    const char      *cp;
    int             result;

    if (!s)
	return 0;
    /* Intentionally the C locale, not the user's locale */
    for (cp = s->str_text; *cp && isprint((unsigned char)*cp); ++cp)
	;
    result = (cp - s->str_text);
    if (result > max)
	result = max;
    return result;
}


void
get_change_aetar(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    string_ty       *qp;
    string_ty       *command;
    int             entire_source = 0;
    size_t          j;
    string_ty       *s1;
    string_ty       *s2;

    /*
     * see of the specified any options.
     */
    for (j = 1; j < modifier->nstrings; ++j)
    {
	const char      *s;

	s = modifier->string[j]->str_text;
	if (0 == strcasecmp(s, "es"))
	    entire_source = 1;
    }

    /*
     * Build the command to be executed.
     */
    qp = str_quote_shell(project_name_get(cp->pp));
    command =
	str_format
	(
	    "%s/aetar -send %s -p %s -c %ld\n",
	    configured_bindir(),
	    (entire_source ? "-es" : ""),
	    qp->str_text,
	    cp->number
	);
    str_free(qp);

    /*
     * The aetar command does not add a MIME header, so we have to.
     */
    printf("MIME-Version: 1.0\n");
    printf("Content-Type: application/x-tar\n");
    s1 = project_name_get(cp->pp);
    if (entire_source)
	s2 = project_description_get(cp->pp);
    else
	s2 = change_brief_description_get(cp);
    printf
    (
	"Subject: %.*s - %.*s\n",
	len_printable(s1, 40),
	s1->str_text,
	len_printable(s2, 80),
	s2->str_text
    );
    if (!entire_source)
    {
	printf
	(
	    "Content-Name: %s.C%3.3ld.tar.gz\n",
	    project_name_get(cp->pp)->str_text,
	    cp->number
	);
	printf
	(
	    "Content-Disposition: attachment; filename=%s.C%3.3ld.tar.gz\n",
	    project_name_get(cp->pp)->str_text,
	    cp->number
	);
    }
    else
    {
	printf
	(
	    "Content-Name: %s.tar.gz\n",
	    project_name_get(cp->pp)->str_text
	);
	printf
	(
	    "Content-Disposition: attachment; filename=%s.tar.gz\n",
	    project_name_get(cp->pp)->str_text
	);
    }
    printf("\n");
    fflush(stdout);

    /*
     * Run the command.
     * This will exec() and does not return.
     */
    get_command(command->str_text);
    /* NOTREACHED */
}
