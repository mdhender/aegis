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
#include <aemakegen/process_item/libdir.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_libdir::~process_item_libdir()
{
}


process_item_libdir::process_item_libdir(target &a_tgt) :
    process_item(a_tgt, &target::process_item_libdir)
{
}


process_item_libdir::pointer
process_item_libdir::create(target &a_tgt)
{
    return pointer(new process_item_libdir(a_tgt));
}


bool
process_item_libdir::condition(const nstring &fn)
{
    return fn.starts_with("libdir/");
}


void
process_item_libdir::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_seen_libdir();

    nstring stem = fn.trim_first_directory();
    nstring dst = "$(libdir)/" + stem;
    data.remember_install_libdir(dst);

    data.set_install_data_macro();
}


// vim: set ts=8 sw=4 et :
