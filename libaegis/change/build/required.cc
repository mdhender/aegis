//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
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
// MANIFEST: implementation of the change_build_required class
//

#include <common/error.h> // for assert
#include <common/nstring.h>
#include <libaegis/change.h>
#include <libaegis/pconf.h>


bool
change_build_required(change_ty *cp)
{
    static nstring exit_0 = "exit 0";
    pconf_ty *pconf_data = change_pconf_get(cp, 1);
    assert(pconf_data);
    return (nstring(pconf_data->development_build_command) != exit_0);
}
