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

#include <common/error.h>

#include <aemakegen/process_data.h>
#include <aemakegen/process_item/yacc.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_yacc::~process_item_yacc()
{
}


process_item_yacc::process_item_yacc(target &a_tgt) :
    process_item(a_tgt, &target::process_item_yacc)
{
}


process_item_yacc::pointer
process_item_yacc::create(target &a_tgt)
{
    return pointer(new process_item_yacc(a_tgt));
}


bool
process_item_yacc::condition(const nstring &filename)
{
    // pathological case
    if (filename.basename() == ".y")
        return false;

    return filename.ends_with(".y");
}


void
process_item_yacc::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_need_yacc();
    if (!data.have_yacc())
    {
        fatal_raw
        (
            "%s: the configure.ac file fails to invoke \"AC_PROG_YACC\" "
            "and yet the project source files would appear to require it",
            fn.c_str()
        );
    }
    data.remember_source_file(fn);

    nstring stem = fn.trim_extension();
    nstring oc = stem + ".yacc.c";
    if (data.seen_c_plus_plus())
        oc += "c";
    nstring oh = stem + ".yacc.h";
    nstring ol = stem + ".list";

    data.remember_clean_obj_file(oc);
    data.remember_clean_obj_file(oh);
    data.remember_clean_obj_file(ol);
    tgt_preprocess(oc);
    tgt_preprocess(oh);
    data.remember_built_sources(oc);
    data.remember_built_sources(oh);

    // for cleaning up afterwards
    data.remember_clean_obj_file("y.output");
    data.remember_clean_obj_file("y.tab.c");
    data.remember_clean_obj_file("y.tab.h");
}


// vim: set ts=8 sw=4 et :
