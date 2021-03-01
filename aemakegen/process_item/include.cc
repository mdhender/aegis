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
#include <aemakegen/process_item/include.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_include::~process_item_include()
{
}


process_item_include::process_item_include(target &a_tgt) :
    process_item(a_tgt, &target::process_item_include)
{
}


process_item_include::pointer
process_item_include::create(target &a_tgt)
{
    return pointer(new process_item_include(a_tgt));
}


bool
process_item_include::condition(const nstring &fn)
{
    // Note: the 'install_include-sources' list is worked out *before* any
    // preprocessing is done.
    return
        (
            data.get_install_include_sources().member(fn)
        ||
            is_an_include_file(fn)
        );
}


void
process_item_include::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.remember_include_files_by_dir(fn);

    if (data.get_install_include_sources().member(fn))
    {
        nstring dst = "$(includedir)/" + fn;
        if (data.get_install_include_sources().size() == 1)
            dst = "$(includedir)/" + fn.trim_first_directory();
        else if (data.get_library_directory() == "lib")
        {
            dst =
                (
                    "$(includedir)/"
                +
                    get_project_name()
                +
                    "/"
                +
                    fn.trim_first_directory()
                );
        }

        data.remember_install_include(dst);
        data.set_install_data_macro();
    }
}


// vim: set ts=8 sw=4 et :
