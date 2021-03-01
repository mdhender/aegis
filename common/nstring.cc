//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the nstring class
//

#include <common/nstring.h>


nstring
nstring::format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    nstring result(str_vformat(fmt, ap));
    va_end(ap);
    return result;
}


nstring
nstring::vformat(const char *fmt, va_list ap)
{
    return nstring(str_vformat(fmt, ap));
}


string_ty *
nstring::get_empty_ref()
{
    return str_from_c("");
}
