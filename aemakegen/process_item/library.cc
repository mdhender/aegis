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
#include <aemakegen/process_item/library.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_library::~process_item_library()
{
}


process_item_library::process_item_library(target &a_tgt) :
    process_item(a_tgt, &target::process_item_library)
{
}


process_item_library::pointer
process_item_library::create(target &a_tgt)
{
    return pointer(new process_item_library(a_tgt));
}


bool
process_item_library::condition(const nstring &)
{
    return false;
}


void
process_item_library::preprocess(const nstring &)
{
}


void
process_item_library::preprocess_end(void)
{
    nstring_list dirs = data.get_list_of_library_directories();
    for (size_t j = 0; j < dirs.size(); ++j)
    {
        nstring library_directory_name = dirs[j];
        nstring library_libname = library_directory_name;
        if (library_libname == "lib")
            library_libname = get_project_name();
        if (!library_libname.starts_with("lib"))
            library_libname = "lib" + library_libname;
        nstring filename =
            (
                library_directory_name
            +
                "/"
            +
                library_libname
            +
                "."
            +
                data.libext()
            );
        data.remember_clean_obj_file(filename);

        // for shared libraries, also cean up libtool crap
        if (data.use_libtool())
            data.remember_dist_clean_dir(library_directory_name + "/.libs");
    }
}


// vim: set ts=8 sw=4 et :
