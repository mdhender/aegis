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
#include <aemakegen/process_item/test_base64.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_test_base64::~process_item_test_base64()
{
}


process_item_test_base64::process_item_test_base64(target &a_tgt) :
    process_item(a_tgt, &target::process_item_aegis_test_base64)
{
}


process_item_test_base64::pointer
process_item_test_base64::create(target &a_tgt)
{
    return pointer(new process_item_test_base64(a_tgt));
}


bool
process_item_test_base64::condition(const nstring &fn)
{
    // pathological case
    if (fn.basename() == ".uue")
        return false;

    return fn.ends_with(".uue");
}


void
process_item_test_base64::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.remember_source_file(fn);
    nstring fn2 = fn.trim_extension();
    data.remember_clean_misc_file(fn2);
    data.remember_built_sources(fn2);

    // now preprocess the result of the rule.
    // it could be just about anything.
    tgt_preprocess(fn2);
}


// vim: set ts=8 sw=4 et :
