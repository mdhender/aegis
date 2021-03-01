//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#include <aemeasure/end_with.h>
#include <aemeasure/getc_special.h>
#include <aemeasure/lang/roff.h>


static void
roff_count(FILE *fp, lang_data_t *data)
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

	case '.':
	case '\'':
	    data->line_has_code = 1;
	    if (data->line_length > 1)
		continue;
	    for (;;)
	    {
		c = getc_special(fp, data);
		switch (c)
		{
		case ' ':
		case '\t':
		    continue;

		default:
		    break;
		}
		break;
	    }
	    if (c != '\\')
		continue;
	    c = getc_special(fp, data);
	    if (c != '"')
		continue;
	    data->line_has_code = 0;
	    data->line_has_comment = 1;
	    for (;;)
	    {
		c = getc_special(fp, data);
		if (c == EOF || c == '\n')
		    break;
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
roff_test(const char *filename)
{
    static const char *list[] =
    {
	".so", ".man", ".mm", ".ms", ".1", ".2", ".3", ".4", ".5", ".6",
	".7", ".8", ".9", ".n", ".l", 0
    };
    return ends_with_one_of(filename, list);
}


lang_t lang_roff =
{
    "roff",
    roff_count,
    roff_test,
};
