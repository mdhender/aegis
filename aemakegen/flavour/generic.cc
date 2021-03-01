//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <aemakegen/flavour/generic.h>
#include <aemakegen/process_item/include.h>
#include <aemakegen/process_item/uudecode.h>


flavour_generic::~flavour_generic()
{
}


flavour_generic::flavour_generic(target &a_tgt) :
    flavour(a_tgt),
    tgt2(a_tgt)
{
}


flavour_generic::pointer
flavour_generic::create(target &a_tgt)
{
    return pointer(new flavour_generic(a_tgt));
}


void
flavour_generic::set_process(void)
{
    register_process(process_item_uudecode::create(tgt2));
    flavour::set_process();
    register_process(process_item_include::create(tgt2));
}


// vim: set ts=8 sw=4 et :
