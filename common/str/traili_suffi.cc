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
// MANIFEST: implementation of the str_traili_suffi class
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
    	!memcmp
	(
	    haystack->str_text + haystack->str_length - needle->str_length,
	    needle->str_text,
	    needle->str_length
	)
    );
}
