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
#include <aemakegen/process_item/etc_man.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


#define USE_DEBIAN 1


process_item_etc_man::~process_item_etc_man()
{
}


process_item_etc_man::process_item_etc_man(target &a_tgt) :
    process_item(a_tgt, &target::process_item_etc_man)
{
}


process_item_etc_man::pointer
process_item_etc_man::create(target &a_tgt)
{
    return pointer(new process_item_etc_man(a_tgt));
}


bool
process_item_etc_man::condition(const nstring &fn)
{
    return fn.gmatch("etc/*.man");
}


void
process_item_etc_man::preprocess(const nstring &fn)
{
    nstring pdf = fn.trim_extension() + ".pdf";
    data.remember_all_doc(pdf);
    data.remember_clean_misc_file(pdf);
    data.set_need_groff();
    data.set_install_data_macro();

    // for the ps2pdf package
    data.set_need_ghostscript();

#if USE_DEBIAN
    // this is what Debian wants
    nstring dst =
        "$(datarootdir)/doc/" + get_project_name() + "/" +
        pdf.trim_first_directory();
#else
    // this is what I'd prefer
    nstring dst =
        "$(datadir)/" + get_project_name() + "/" + pdf.trim_first_directory();
#endif
    data.remember_install_doc(dst);
}


// vim: set ts=8 sw=4 et :
