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
#include <aemakegen/process_item/fmtgen.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_fmtgen::~process_item_fmtgen()
{
}


process_item_fmtgen::process_item_fmtgen(target &a_tgt) :
    process_item(a_tgt, &target::process_item_aegis_fmtgen)
{
}


process_item_fmtgen::pointer
process_item_fmtgen::create(target &a_tgt)
{
    return pointer(new process_item_fmtgen(a_tgt));
}


bool
process_item_fmtgen::condition(const nstring &fn)
{
    // pathological case
    if (fn.basename() == ".def")
        return false;

    return fn.ends_with(".def");
}


void
process_item_fmtgen::preprocess(const nstring &fn)
{
    assert(condition(fn));

    // From the Automake manual, section 18.2, "Handling new file extensions":
    // you include the source file in the list of source files, even though it
    // is a source file that Automake doesn't grok.
    data.remember_source_file(fn);

    nstring stem = fn.trim_extension();
    nstring oc = stem + ".fmtgen.cc";
    nstring oh = stem + ".fmtgen.h";
    data.remember_clean_obj_file(oc);
    data.remember_clean_obj_file(oh);
    data.remember_built_sources(oc);
    data.remember_built_sources(oh);

    // and also the output
    tgt_preprocess(oc);
    tgt_preprocess(oh);
}


void
process_item_fmtgen::preprocess_end(void)
{
    // all the object for the common/libcommon.a library
    // are to be included in the libaegis/libaegis.a library.
    // (This is the least ugly way to do this.)
    data.library_plus_library("libaegis", "common");

    // The fmtgen command uses common/libcommon.a instead of
    // libaegis/libaegis.a because it is sued to build some of the
    // contents of libaegis/libaegis.a
    data.program_needs_library("fmtgen", "common");
}


// vim: set ts=8 sw=4 et :
