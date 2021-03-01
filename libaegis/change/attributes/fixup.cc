//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller
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

#include <common/ac/ctype.h>

#include <libaegis/cattr.h>
#include <libaegis/change/attributes.h>


void
change_attributes_fixup(cattr_ty *cap)
{
    if (cap->description)
    {
	string_ty *s = cap->description;
	cap->description = str_snip(s);
	str_free(s);
    }

    if (cap->brief_description)
    {
	string_ty *s = cap->brief_description;
	cap->brief_description = str_snip(s);
	str_free(s);
    }
}
