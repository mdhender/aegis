//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/nstring.h>
#include <libaegis/change.h>
#include <libaegis/pconf.fmtgen.h>


bool
change_build_required(change::pointer cp, bool conf_must_exist)
{
    static nstring exit_0 = "exit 0";
    pconf_ty *pconf_data = change_pconf_get(cp, conf_must_exist);
    assert(pconf_data);
    return (nstring(pconf_data->development_build_command) != exit_0);
}


// vim: set ts=8 sw=4 et :
