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
#include <aemakegen/process_item/aegis/lib_doc.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_aegis_lib_doc::~process_item_aegis_lib_doc()
{
}


process_item_aegis_lib_doc::process_item_aegis_lib_doc(target &a_tgt) :
    process_item(a_tgt, &target::process_item_aegis_lib_doc)
{
}


process_item_aegis_lib_doc::pointer
process_item_aegis_lib_doc::create(target &a_tgt)
{
    return pointer(new process_item_aegis_lib_doc(a_tgt));
}


bool
process_item_aegis_lib_doc::condition(const nstring &fn)
{
    nstring_list parts;
    parts.split(fn, "/");
    return
        (
            parts.size() == 4
        &&
            parts[0] == "lib"
        &&
            parts[3].starts_with("main.")
        );
}


void
process_item_aegis_lib_doc::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_seen_datarootdir();
    data.set_need_groff();
    data.set_need_ghostscript(); // for ps2pdf

    nstring macros = fn.get_extension();
    if (macros == "mm")
        data.set_groff_macro();
    if (macros == "ms")
        data.set_groff_macro();

    nstring pdf = fn.dirname() + ".pdf";
    data.remember_all_doc(pdf);
    data.remember_am_data_data(pdf);
    data.remember_clean_misc_file(pdf);

    nstring_list part;
    part.split(fn, "/");
    nstring dst = "$(datarootdir)/doc/" + get_project_name() + "/";
    if (part[1] != "en")
        dst += part[1] + "/";
    dst += part[2] + ".pdf";
    data.remember_install_doc(dst);
}


// vim: set ts=8 sw=4 et :
