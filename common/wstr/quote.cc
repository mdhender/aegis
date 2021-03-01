//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004-2006, 2008 Peter Miller
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

#include <common/wstr.h>
#include <common/str.h>


wstring_ty *
wstr_quote_shell(wstring_ty *wsp)
{
    string_ty       *s;

    s = wstr_to_str(wsp);
    s = str_quote_shell(s);
    wsp = str_to_wstr(s);
    str_free(s);
    return wsp;
}
