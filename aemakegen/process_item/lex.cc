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
#include <aemakegen/process_item/lex.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_lex::~process_item_lex()
{
}


process_item_lex::process_item_lex(target &a_tgt) :
    process_item(a_tgt, &target::process_item_lex)
{
}


process_item_lex::pointer
process_item_lex::create(target &a_tgt)
{
    return pointer(new process_item_lex(a_tgt));
}


bool
process_item_lex::condition(const nstring &filename)
{
    // pathological case
    if (filename.basename() == ".l")
        return false;

    return filename.ends_with(".l");
}


void
process_item_lex::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_need_lex();
    if (!data.have_lex())
    {
        fatal_raw
        (
            "%s: the configure.ac file fails to invoke \"AC_PROG_LEX\" "
            "and yet the project source files would appear to require it",
            fn.c_str()
        );
    }
    data.remember_source_file(fn);
    nstring stem = fn.trim_extension();

    nstring oc = stem + ".lex.c";
    if (data.seen_c_plus_plus())
        oc += "c";

    data.remember_clean_obj_file(oc);
    tgt_preprocess(oc);
    data.remember_built_sources(oc);

    // for cleaning up afterwards
    data.remember_clean_obj_file("lex.yy.c");
}


// vim: set ts=8 sw=4 et :
