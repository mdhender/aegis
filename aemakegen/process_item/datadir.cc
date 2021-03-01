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
#include <aemakegen/process_item/datadir.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_datadir::~process_item_datadir()
{
}


process_item_datadir::process_item_datadir(target &a_tgt) :
    process_item(a_tgt, &target::process_item_datadir)
{
}


process_item_datadir::pointer
process_item_datadir::create(target &a_tgt)
{
    return pointer(new process_item_datadir(a_tgt));
}


bool
process_item_datadir::condition(const nstring &fn)
{
    return fn.starts_with("datadir/");
}


void
process_item_datadir::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_seen_datadir();

    nstring stem = fn.substr(8, fn.size() - 8);
    nstring dst = "$(datadir)/" + get_project_name() + "/" + stem;
    data.remember_install_datadir(dst);

    data.set_install_data_macro();
}


// vim: set ts=8 sw=4 et :
