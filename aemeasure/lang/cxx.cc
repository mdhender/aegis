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
// MANIFEST: functions to manipulate cxxs
//

#include <aemeasure/end_with.h>
#include <aemeasure/getc_special.h>
#include <aemeasure/lang/cxx.h>


static void
cxx_count(FILE *fp, lang_data_t *data)
{
    for (;;)
    {
	int             c;

	c = getc_special(fp, data);
	loop:
	switch (c)
	{
	case EOF:
	    break;

	case ' ':
	case '\b':
	case '\f':
	case '\n':
	case '\r':
	case '\t':
	case '\v':
	    continue;

	case '/':
	    c = getc_special(fp, data);
	    if (c == EOF)
		break;
	    if (c == '/')
	    {
		// C++ comment
		data->line_has_comment = 1;
		for (;;)
		{
		    c = getc_special(fp, data);
		    if (c == EOF || c == '\n')
			break;
		}
		continue;
	    }
	    if (c == '*')
	    {
		// C comment
		for (;;)
		{
		    for (;;)
		    {
			data->line_has_comment = 1;
			c = getc_special(fp, data);
			if (c == EOF || c == '*')
			    break;
		    }
		    for (;;)
		    {
			data->line_has_comment = 1;
			c = getc_special(fp, data);
			if (c != '*')
			    break;
		    }
		    if (c == EOF || c == '/')
			break;
		}
		continue;
	    }
	    if (c == '/')
	    {
		// C++ comment
		data->line_has_comment = 1;
		for (;;)
		{
		    c = getc_special(fp, data);
		    if (c == EOF || c == '\n')
			break;
		}
		continue;
	    }
	    data->line_has_code = 1;
	    goto loop;

	case '"':
	    for (;;)
	    {
		data->line_has_code = 1;
		c = getc_special(fp, data);
		if (c == EOF || c == '"' || c == '\n')
		    break;
		if (c == '\\')
		    getc_special(fp, data);
	    }
	    continue;

	case '\'':
	    for (;;)
	    {
		data->line_has_code = 1;
		c = getc_special(fp, data);
		if (c == EOF || c == '\'' || c == '\n')
		    break;
		if (c == '\\')
		    getc_special(fp, data);
	    }
	    continue;

	default:
	    data->line_has_code = 1;
	    continue;
	}
	break;
    }
}


static int
cxx_test(const char *filename)
{
    static const char *list[] =
    {
	".C", ".cc", ".CC", ".c++",
	".H", ".hh", ".HH", ".h++",
	0
    };
    return ends_with_one_of(filename, list);
}


lang_t lang_cxx =
{
    "c++",
    cxx_count,
    cxx_test,
};
