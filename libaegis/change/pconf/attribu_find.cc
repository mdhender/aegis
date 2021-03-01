//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/nstring.h>
#include <libaegis/attribute.h>
#include <libaegis/change.h>


nstring
change::pconf_attributes_find(const nstring &name)
{
    pconf_ty *pcp = change_pconf_get(this, 0);
    assert(pcp);
    if (!pcp)
	return "";
    attributes_ty *ap =
	attributes_list_find(pcp->project_specific, name.c_str());
    if (!ap)
	return "";
    assert(ap->value);
    if (!ap->value)
	return "";
    return nstring(ap->value);
}
