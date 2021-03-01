//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <common/str.h>


string_ty *
str_snip(string_ty *s)
{
    const char *ep = s->str_text + s->str_length;
    while (ep > s->str_text && isspace((unsigned char)ep[-1]))
	--ep;
    const char *cp = s->str_text;
    while (cp < ep && isspace((unsigned char)*cp))
	++cp;
    return str_n_from_c(cp, ep - cp);
}
