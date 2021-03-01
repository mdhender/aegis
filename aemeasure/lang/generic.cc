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
// MANIFEST: functions to manipulate generics
//

#include <aemeasure/getc_special.h>
#include <aemeasure/lang/generic.h>


static void
generic_count(FILE *fp, lang_data_t *data)
{
    for (;;)
    {
	int             c;

	c = getc_special(fp, data);
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

	case '#':
	    data->line_has_comment = 1;
	    continue;

	default:
	    data->line_has_code = 1;
	    continue;
	}
	break;
    }
}


static int
generic_test(const char *filename)
{
    return 1;
}


lang_t lang_generic =
{
    "generic",
    generic_count,
    generic_test,
};
