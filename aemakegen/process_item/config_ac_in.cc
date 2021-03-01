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

#include <common/ac/assert.h>

#include <aemakegen/process_data.h>
#include <aemakegen/process_item/config_ac_in.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_configure_ac_in::~process_item_configure_ac_in()
{
}


process_item_configure_ac_in::process_item_configure_ac_in(target &a_tgt) :
    process_item(a_tgt, &target::process_item_configure_ac_in)
{
}


process_item_configure_ac_in::pointer
process_item_configure_ac_in::create(target &a_tgt)
{
    return pointer(new process_item_configure_ac_in(a_tgt));
}


bool
process_item_configure_ac_in::condition(const nstring &fn)
{
    return (fn.basename() == "configure.ac.in");
}


void
process_item_configure_ac_in::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.remember_source_file(fn);
    nstring fn2 = fn.trim_extension();
    data.remember_dist_clean_file(fn2);
    data.remember_built_sources(fn2);
    data.remember_extra_dist(fn);

    // now preprocess the result of the rule.
    tgt_preprocess(fn2);
}


// vim: set ts=8 sw=4 et :
