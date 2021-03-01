//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#include <common/mprintf.h>
#include <common/str.h>


string_ty *
str_format(const char *fmt, ...)
{
    va_list	    ap;
    string_ty	    *result;

    va_start(ap, fmt);
    result = vmprintf_str(fmt, ap);
    va_end(ap);
    return result;
}


string_ty *
str_vformat(const char *fmt, va_list ap)
{
    return vmprintf_str(fmt, ap);
}
