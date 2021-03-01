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
#include <aemakegen/process_item/aegis/lib_junk.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_aegis_lib_junk::~process_item_aegis_lib_junk()
{
}


process_item_aegis_lib_junk::process_item_aegis_lib_junk(target &a_tgt) :
    process_item(a_tgt, &target::process_item_aegis_lib_junk)
{
}


process_item_aegis_lib_junk::pointer
process_item_aegis_lib_junk::create(target &a_tgt)
{
    return pointer(new process_item_aegis_lib_junk(a_tgt));
}


bool
process_item_aegis_lib_junk::condition(const nstring &fn)
{
    return
        (
            fn.gmatch("lib/*/man?/*")
        ||
            fn.gmatch("lib/*/*/*.so")
        ||
            fn.gmatch("lib/*/*/*.bib")
        );
}


void
process_item_aegis_lib_junk::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.remember_extra_dist(fn);
}


// vim: set ts=8 sw=4 et :
