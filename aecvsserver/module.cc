//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
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

#include <aecvsserver/file_info.h>
#include <aecvsserver/module.h>
#include <aecvsserver/server.h>


module_ty::~module_ty()
{
    assert(reference_count == 1);
    reference_count = -666;
    if (canonical_name)
    {
        str_free(canonical_name);
        canonical_name = 0;
    }
}


module_ty::module_ty() :
    reference_count(1),
    canonical_name(0)
{
}


void
module_ty::reference_count_down()
{
    assert(reference_count >= 1);
    if (reference_count <= 1)
        delete this;
    reference_count--;
}


void
module_ty::reference_count_up()
{
    assert(reference_count >= 1);
    reference_count++;
}


string_ty *
module_ty::name()
{
    if (!canonical_name)
    {
        canonical_name = calculate_canonical_name();
    }
    return canonical_name;
}


void
module_ty::checkout(server_ty *sp)
{
    options opt;
    opt.d = 1;
    opt.C = 1;
    if (update(sp, name(), name(), opt))
        server_ok(sp);
}


bool
module_ty::is_bogus()
    const
{
    return false;
}


// vim: set ts=8 sw=4 et :
