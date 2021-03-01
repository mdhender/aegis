//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/glue.h>
#include <common/nstring/list.h>


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
	result->push_back_unique(s);
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
    result->clear();
    for (cp = data; cp < data + data_len; )
    {
	string_ty       *s;

	s = str_from_c(cp);
	cp += s->str_length + 1;
	result->push_back(s);
	str_free(s);
    }
    // do NOT free what data is pointing to
    return 0;
}


int
read_whole_dir__wla(const nstring &path, nstring_list &result)
{
    char *data = 0;
    long data_len = 0;
    if (glue_read_whole_dir(path.c_str(), &data, &data_len))
	return -1;
    for (char *cp = data; cp < data + data_len; )
    {
	nstring s(cp);
	cp += s.size() + 1;
	result.push_back_unique(s);
    }
    // do NOT free what data is pointing to
    return 0;
}


int
read_whole_dir__wl(const nstring &path, nstring_list &result)
{
    result.clear();
    return read_whole_dir__wla(path, result);
}
