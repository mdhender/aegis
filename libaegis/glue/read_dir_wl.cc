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
// MANIFEST: functions to manipulate read_dir_wls
//

#include <glue.h>
#include <str_list.h>


int
read_whole_dir__wla(const char *path, string_list_ty *result)
{
    char            *data;
    long            data_len;
    char            *cp;

    if (glue_read_whole_dir(path, &data, &data_len))
	return -1;
    for (cp = data; cp < data + data_len; )
    {
	string_ty       *s;

	s = str_from_c(cp);
	cp += s->str_length + 1;
	string_list_append_unique(result, s);
	str_free(s);
    }
    // do NOT free what data is pointing to
    return 0;
}


int
read_whole_dir__wl(const char *path, string_list_ty *result)
{
    char            *data;
    long            data_len;
    char            *cp;

    if (glue_read_whole_dir(path, &data, &data_len))
	return -1;
    string_list_constructor(result);
    for (cp = data; cp < data + data_len; )
    {
	string_ty       *s;

	s = str_from_c(cp);
	cp += s->str_length + 1;
	string_list_append(result, s);
	str_free(s);
    }
    // do NOT free what data is pointing to
    return 0;
}
