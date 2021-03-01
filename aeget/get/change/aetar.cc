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
// MANIFEST: functions to manipulate aetars
//

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/string.h>

#include <change.h>
#include <change/branch.h>
#include <cstate.h>
#include <get/change/aetar.h>
#include <get/command.h>
#include <libdir.h>
#include <nstring.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>


static int
len_printable(string_ty *s, int max)
{
    const char      *cp;
    int             result;

    if (!s)
	return 0;
    // Intentionally the C locale, not the user's locale
    for (cp = s->str_text; *cp && isprint((unsigned char)*cp); ++cp)
	;
    result = (cp - s->str_text);
    if (result > max)
	result = max;
    return result;
}


static nstring
fixup(const char *s)
{
    char buffer[256];
    char *bp = buffer;
    while (*s && bp < buffer + sizeof(buffer) - 1)
    {
	unsigned char c = (unsigned char)*s++;
	switch (c)
	{
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '-': case '.': case '_':
	    break;

	default:
	    c = '_';
	    break;
	}
	*bp++ = c;
    }
    return nstring(buffer, bp - buffer);

}


void
get_change_aetar(change_ty *cp, string_ty *fn, string_list_ty *modifier)
{
    //
    // See if they specified any options.
    //
    bool entire_source = false;
    bool noprefix = false;
    nstring prefix;
    for (size_t j = 0; j < modifier->nstrings; ++j)
    {
	const char *s = modifier->string[j]->str_text;
	if (0 == strcasecmp(s, "es"))
	    entire_source = true;
	if (0 == strcasecmp(s, "noprefix"))
	    noprefix = true;
	if (0 == strncasecmp(s, "prefix=", 7))
	{
	    prefix = fixup(s + 7);
	    noprefix = true; // so can specify empty prefix
	}
    }
    if (prefix.empty() && !noprefix)
    {
	//
	// default the prefix
	//
	project_ty *pp = cp->pp;
	while (pp->parent)
	    pp = pp->parent;
	nstring proj = fixup(project_name_get(pp)->str_text);
	nstring rev(change_version_get(cp));
	prefix = proj + "." + rev;
    }

    //
    // Build the command to be executed.
    //
    if (!prefix.empty())
	prefix = " -add-path-prefix=" + prefix;
    nstring quoted_project(str_quote_shell(project_name_get(cp->pp)));
    nstring command =
	nstring::format
	(
	    "%s/aetar -send %s%s -p %s -c %ld\n",
	    configured_bindir(),
	    (entire_source ? "-es" : ""),
	    prefix.c_str(),
	    quoted_project.c_str(),
	    magic_zero_decode(cp->number)
	);

    //
    // The aetar command does not add a MIME header, so we have to.
    //
    printf("MIME-Version: 1.0\n");
    printf("Content-Type: application/x-tar\n");
    string_ty *s1 = project_name_get(cp->pp);
    string_ty *s2;
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
	    magic_zero_decode(cp->number)
	);
	printf
	(
	    "Content-Disposition: attachment; filename=%s.C%3.3ld.tar.gz\n",
	    project_name_get(cp->pp)->str_text,
	    magic_zero_decode(cp->number)
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

    //
    // Run the command.
    // This will exec() and does not return.
    //
    get_command(command.c_str());
    // NOTREACHED
}
