//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Walter Franzini
//	Copyright (C) 2008 Peter Miller
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

#include <common/str.h>
#include <libaegis/os.h>

nstring
os_canonify_dirname(const nstring &dirname)
{
    if (dirname[dirname.length() - 1] != '/')
        return dirname;
    return nstring(dirname.c_str(), dirname.length() - 1);
}

string_ty *
os_canonify_dirname(string_ty *dirname)
{
    if (dirname->str_text[dirname->str_length - 1] != '/')
        return dirname;
    return str_n_from_c(dirname->str_text, dirname->str_length - 1);
}
