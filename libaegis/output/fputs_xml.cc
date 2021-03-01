//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the output_ty::fputs_xml methods
//

#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <libaegis/output.h>


static bool
near_end(const char *s)
{
    for (;;)
    {
	unsigned char c = *s++;
	if (!c)
	    return true;
	if (!isspace(c))
	    return false;
    }
}



void
output_ty::fputs_xml(const char *s, bool paragraphs)
{
    if (!s)
	return;
    for (;;)
    {
	unsigned char c = *s++;
	switch (c)
	{
	case '\0':
	    return;

	case '<':
	    fputs("&lt;");
	    break;

	case '>':
	    fputs("&gt;");
	    break;

	case '&':
	    fputs("&amp;");
	    break;

	case '"':
	    fputs("&quot;");
	    break;

	case '\n':
	    if (paragraphs)
	    {
		if (near_end(s))
		    return;
		if (*s == '\n')
		    fputs("\n<p>");
		else
		    fputs("<br>");
	    }
	    fputc('\n');
	    while (*s == '\n')
		++s;
	    break;

	default:
	    // always in the C locale
	    if (isprint(c))
		fputc(c);
	    else
		fprintf("&#%d;", c);
	    break;
	}
    }
}


void
output_ty::fputs_xml(string_ty *s, bool para)
{
    if (!s)
	return;
    fputs_xml(s->str_text, para);
}


void
output_ty::fputs_xml(const nstring &s, bool para)
{
    fputs_xml(s.c_str(), para);
}
