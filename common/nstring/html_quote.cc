//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the html_text_quote class
//

#include <ac/ctype.h>
#include <ac/stdio.h> // for snprintf
#include <ac/string.h>

#include <nstring.h>
#include <nstring/accumulator.h>


static bool
ends_soon(const char *s)
{
    for (;;)
    {
	if (!*s)
	    return true;
	if (!isspace((unsigned char)*s))
	    return false;
	++s;
    }
}


nstring
nstring::html_quote(bool paragraphs)
    const
{
    const char *s = c_str();
    int column = 0;
    static nstring_accumulator sa;
    sa.clear();
    for (;;)
    {
	unsigned char c = *s++;
	switch (c)
	{
	case '\n':
	    if (ends_soon(s))
	    {
	case 0:
		return sa.mkstr();
	    }
	    if (paragraphs)
	    {
		if (*s == '\n')
		{
		    ++s;
		    sa.push_back("\n<p>\n");
		}
		else
		{
		    sa.push_back("<br>\n");
		}
		while (*s && isspace((unsigned char)*s))
		    ++s;
	    }
	    else
	    {
		sa.push_back('\n');
	    }
	    column = 0;
	    break;

	case '&':
	    sa.push_back("&amp;");
	    column += 5;
	    break;

	case '<':
	    sa.push_back("&lt;");
	    column += 4;
	    break;

	case '>':
	    sa.push_back("&gt;");
	    column += 4;
	    break;

	case '"':
	    sa.push_back("&quot;");
	    column += 6;
	    break;

	case ' ':
	case '\t':
	    //
	    // HTTP only allows lines of up to 510 characters,
	    // so break the line once it gets too wide.
	    //
	    if (column > 70)
	    {
		while (*s && isspace((unsigned char)*s))
		    ++s;
		sa.push_back('\n');
		column = 0;
	    }
	    else
	    {
		sa.push_back(c);
		++column;
	    }
	    break;

	default:
	    // C locale
	    if (isprint(c))
	    {
		sa.push_back(c);
		++column;
	    }
	    else
	    {
		char temp[10];
		snprintf(temp, sizeof(temp), "&#%d;", c);
		size_t len = strlen(temp);
		sa.push_back(temp, len);
		column += len;
	    }
	    break;
	}
    }
}
