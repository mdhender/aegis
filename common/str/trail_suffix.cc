//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2008 Walter Franzini
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
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>
#include <common/str.h>

int
str_trailing_suffix(string_ty *haystack, string_ty *needle)
{
    return
    (
        haystack->str_length >= needle->str_length
    &&
        0 ==
        memcmp
        (
            haystack->str_text + (haystack->str_length - needle->str_length),
            needle->str_text,
            needle->str_length
        )
    );
}
