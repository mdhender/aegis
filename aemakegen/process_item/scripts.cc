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
#include <aemakegen/process_item/scripts.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_scripts::~process_item_scripts()
{
}


process_item_scripts::process_item_scripts(target &a_tgt) :
    process_item(a_tgt, &target::process_item_scripts)
{
}


process_item_scripts::pointer
process_item_scripts::create(target &a_tgt)
{
    return pointer(new process_item_scripts(a_tgt));
}


bool
process_item_scripts::condition(const nstring &fn)
{
    // make sure that stuff like "00README.txt" isn't installed as a
    // script, because it isn't.
    return filename_implies_is_a_script(fn);
}


void
process_item_scripts::preprocess(const nstring &fn)
{
    assert(condition(fn));

    nstring name = fn.basename();
    nstring ext = name.get_extension();
    if (extension_implies_script(name))
        name = name.trim_extension();

    nstring bin_name = "bin/" + name + data.exeext();
    data.remember_clean_misc_file(".bin");
    data.remember_all_bin(bin_name);
    if (!data.get_built_sources().member(fn))
        data.remember_extra_dist(fn);

    if (is_installable(name))
    {
        data.set_install_script_macro();
        nstring install_name =
            (
                "$(bindir)/"
            +
                data.get_program_prefix()
            +
                name
            +
                data.get_program_suffix()
            +
                data.exeext()
            );
        data.remember_install_bin(install_name);
    }
}


// vim: set ts=8 sw=4 et :
