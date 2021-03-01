//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the change_attributes_fixup function
//

#include <ac/ctype.h>

#include <cattr.h>
#include <change/attributes.h>


static string_ty *
fixup(string_ty *s)
{
    //
    // We are not using str_trim() because we only want to top and tail
    // it, not clean up the white space im the middle as well.
    //
    const char *ep = s->str_text + s->str_length;
    while (ep > s->str_text && isspace((unsigned char)ep[-1]))
	--ep;
    const char *cp = s->str_text;
    while (cp < ep && isspace((unsigned char)*cp))
	++cp;
    return str_n_from_c(cp, ep - cp);
}


void
change_attributes_fixup(cattr_ty *cap)
{
    if (cap->description)
    {
	string_ty *s = cap->description;
	cap->description = fixup(s);
	str_free(s);
    }

    if (cap->brief_description)
    {
	string_ty *s = cap->brief_description;
	cap->brief_description = fixup(s);
	str_free(s);
    }
}
