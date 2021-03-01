//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate modules
//

#include <common/error.h> // for assert

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
