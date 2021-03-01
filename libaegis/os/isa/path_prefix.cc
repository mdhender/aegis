//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003-2006, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <libaegis/os/isa/path_prefix.h>
#include <common/str.h>


int
os_isa_path_prefix(string_ty *s1, string_ty *s2)
{
	return
	(
		!s1->str_length
	||
		(
			s1->str_length < s2->str_length
		&&
			!memcmp(s1->str_text, s2->str_text, s1->str_length)
		&&
			s2->str_text[s1->str_length] == '/'
		)
	);
}
