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

#include <common/nstring.h>


nstring
nstring::format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string_ty *tmp = str_vformat(fmt, ap);
    nstring result(tmp);
    va_end(ap);
    str_free(tmp);
    return result;
}


nstring
nstring::vformat(const char *fmt, va_list ap)
{
    string_ty *tmp = str_vformat(fmt, ap);
    nstring result(tmp);
    str_free(tmp);
    return result;
}


string_ty *
nstring::get_empty_ref()
{
    return str_from_c("");
}
