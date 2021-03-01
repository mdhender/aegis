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
#include <aemakegen/process_item/etc_msgfm_sh.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_etc_msgfmt_sh::~process_item_etc_msgfmt_sh()
{
}


process_item_etc_msgfmt_sh::process_item_etc_msgfmt_sh(target &a_tgt) :
    process_item(a_tgt, &target::process_item_etc_msgfmt_sh)
{
}


process_item_etc_msgfmt_sh::pointer
process_item_etc_msgfmt_sh::create(target &a_tgt)
{
    return pointer(new process_item_etc_msgfmt_sh(a_tgt));
}


bool
process_item_etc_msgfmt_sh::condition(const nstring &fn)
{
    return (fn == "etc/msgfmt.sh");
}


void
process_item_etc_msgfmt_sh::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_etc_msgfmt_sh();
    data.remember_extra_dist(fn);
}


// vim: set ts=8 sw=4 et :
