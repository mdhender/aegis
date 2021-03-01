//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2003-2006, 2008 Peter Miller
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

#include <libaegis/os.h>


static string_ty *
stripdot(string_ty *s, int strip_left_slash)
{
    const char      *cp;
    size_t          len;

    cp = s->str_text;
    len = s->str_length;
    for (;;)
    {
	if (len > 1 && cp[len - 1] == '/')
	{
	    --len;
	    continue;
	}
	if (len >= 2 && cp[len - 2] == '/' && cp[len - 1] == '.')
	{
	    --len;
	    continue;
	}
	if (len > 2 && cp[0] == '.' && cp[1] == '/')
	{
	    cp += 2;
	    len -= 2;
	    while (*cp == '/')
	    {
		++cp;
		--len;
	    }
	}
	if (strip_left_slash && len >= 1 && cp[0] == '/')
	{
	    ++cp;
	    --len;
	}
	break;
    }
    if (len == 0)
	return str_from_c(".");
    return str_n_from_c(cp, len);
}


string_ty *
os_path_cat(string_ty *s1, string_ty *s2)
{
    static string_ty *dot;
    static string_ty *slash;
    string_ty       *ss1;
    string_ty       *ss2;
    string_ty       *result;

    if (!dot)
	dot = str_from_c(".");
    if (!s1->str_length)
	s1 = dot;
    if (!s2->str_length)
	s2 = dot;
    ss1 = stripdot(s1, 0);
    ss2 = stripdot(s2, 1);
    if (str_equal(ss1, dot))
    {
	str_free(ss1);
	return ss2;
    }
    if (str_equal(ss2, dot))
    {
	str_free(ss2);
	return ss1;
    }

    if (!slash)
	slash = str_from_c("/");
    if (str_equal(ss1, slash))
	result = str_catenate(ss1, ss2);
    else
	result = str_cat_three(ss1, slash, ss2);
    str_free(ss1);
    str_free(ss2);
    return result;
}


static nstring
stripdot(const nstring &s, bool strip_left_slash)
{
    const char *cp = s.c_str();
    size_t len = s.size();
    for (;;)
    {
	if (len > 1 && cp[len - 1] == '/')
	{
	    --len;
	    continue;
	}
	if (len >= 2 && cp[len - 2] == '/' && cp[len - 1] == '.')
	{
	    --len;
	    continue;
	}
	if (len > 2 && cp[0] == '.' && cp[1] == '/')
	{
	    cp += 2;
	    len -= 2;
	    while (*cp == '/')
	    {
		++cp;
		--len;
	    }
	}
	if (strip_left_slash && len >= 1 && cp[0] == '/')
	{
	    ++cp;
	    --len;
	}
	break;
    }
    if (len == 0)
	return ".";
    return nstring(cp, len);
}


nstring
os_path_cat(const nstring &s1, const nstring &s2)
{
    nstring dot(".");
    nstring ss1(stripdot((s1.empty() ? dot : s1), false));
    nstring ss2(stripdot((s2.empty() ? dot : s2), true));
    if (ss1 == dot)
    {
	return ss2;
    }
    if (ss2 == dot)
    {
	return ss1;
    }

    nstring slash("/");
    if (ss1 == slash)
	return (ss1 + ss2);
    return (ss1 + slash + ss2);
}


string_ty *
os_path_rel2abs(string_ty *root, string_ty *path)
{
    if (path->str_text[0] == '/')
       	return str_copy(path);
    return os_path_cat(root, path);
}


nstring
os_path_rel2abs(const nstring &root, const nstring &path)
{
    if (path[0] == '/')
	return path;
    return os_path_cat(root, path);
}
