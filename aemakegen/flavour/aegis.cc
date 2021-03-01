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

#include <aemakegen/flavour/aegis.h>
#include <aemakegen/process_item/aegis/lib_doc.h>
#include <aemakegen/process_item/aegis/lib_else.h>
#include <aemakegen/process_item/aegis/lib_junk.h>
#include <aemakegen/process_item/aegis/lib_sh.h>
#include <aemakegen/process_item/etc_profile.h>
#include <aemakegen/process_item/fmtgen.h>
#include <aemakegen/process_item/lib_icon2.h>
#include <aemakegen/process_item/test_base64.h>


flavour_aegis::~flavour_aegis()
{
}


flavour_aegis::flavour_aegis(target &a_tgt) :
    flavour(a_tgt),
    tgt2(a_tgt)
{
}


flavour_aegis::pointer
flavour_aegis::create(target &a_tgt)
{
    return pointer(new flavour_aegis(a_tgt));
}


void
flavour_aegis::set_process(void)
{
    // we put this one at the front, so it overrides the
    // common "*.uue" processing.
    register_process(process_item_test_base64::create(tgt2));

    // Now do all of the common processes
    flavour::set_process();

    // and now more for Aegis itself
    register_process(process_item_fmtgen::create(tgt2));
    register_process(process_item_lib_icon2::create(tgt2));
    register_process(process_item_etc_profile::create(tgt2));
    register_process(process_item_aegis_lib_sh::create(tgt2));

    // ordering is important here
    register_process(process_item_aegis_lib_junk::create(tgt2));
    register_process(process_item_aegis_lib_doc::create(tgt2));
    register_process(process_item_aegis_lib_else::create(tgt2));
}


// vim: set ts=8 sw=4 et :
