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
#include <aemakegen/process_item/test_sh.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_test_sh::~process_item_test_sh()
{
}


process_item_test_sh::process_item_test_sh(target &a_tgt) :
    process_item(a_tgt, &target::process_item_test_sh)
{
}


process_item_test_sh::pointer
process_item_test_sh::create(target &a_tgt)
{
    return pointer(new process_item_test_sh(a_tgt));
}


bool
process_item_test_sh::condition(const nstring &fn)
{
    if (fn == "etc/test.sh")
    {
        data.set_etc_test_sh();
        return false;
    }
    return fn.gmatch("test/*/*.sh");
}


void
process_item_test_sh::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.remember_test_source(fn);
    nstring stem = fn.substr(0, fn.size() - 3);
    nstring base = stem.basename();

    if (data.seen_etc_test_sh())
    {
        // This code branch is for Aegis itself.
        // (or any project of sufficiently similar shape)
        nstring es = base + ".ES";
        data.remember_test_file(es);
        data.remember_clean_misc_file(es);
    }
    else
    {
        data.remember_test_file(base);
        // no need to clean, it's a .PHONY file
    }
}


// vim: set ts=8 sw=4 et :
