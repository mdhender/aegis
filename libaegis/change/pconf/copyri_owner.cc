//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: implementation of the change::pconf_copyright_owner_get method
//

#include <common/nstring.h>
#include <libaegis/change.h>
#include <libaegis/user.h>


nstring
change::pconf_copyright_owner_get()
{
    nstring value = pconf_attributes_find("copyright-owner");
    if (value.empty())
    {
	user_ty::pointer up = user_ty::create();
	value = up->full_name();
    }
    return value;
}
