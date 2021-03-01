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

#include <aemeasure/getc_special.h>


int
getc_special(FILE *fp, lang_data_t *data)
{
    int             c;

    c = getc(fp);
    if (c != EOF)
	data->length++;
    switch (c)
    {
    case EOF:
	if (data->line_length)
	{
	    if (data->line_has_code)
		data->code_lines++;
	    else if (data->line_has_comment)
		data->comment_lines++;
	    else
		data->blank_lines++;
	}
	break;

    case '\n':
	if (data->line_has_code)
	    data->code_lines++;
	else if (data->line_has_comment)
	    data->comment_lines++;
	else
	    data->blank_lines++;
	data->line_has_code = 0;
	data->line_has_comment = 0;
	data->line_length = 0;
	break;

    case 0:
	data->file_is_binary = 1;
	// fall through...

    default:
	data->line_length++;
	break;
    }
    return c;
}
