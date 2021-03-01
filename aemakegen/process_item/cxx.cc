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
#include <aemakegen/process_item/cxx.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_cxx::~process_item_cxx()
{
}


process_item_cxx::process_item_cxx(target &a_tgt) :
    process_item(a_tgt, &target::process_item_cxx)
{
}


process_item_cxx::pointer
process_item_cxx::create(target &a_tgt)
{
    return pointer(new process_item_cxx(a_tgt));
}


bool
process_item_cxx::condition(const nstring &filename)
{
    return is_a_cxx_source_file(filename);
}


void
process_item_cxx::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_seen_c_plus_plus();

    if
    (
        fn != "y.tab.cc"
    &&
        fn != "lex.yy.cc"
    &&
        !fn.gmatch("*.yacc.cc")
    &&
        !fn.gmatch("*.lex.cc")
    )
    {
        // FIXME: what about *.def ?
        data.remember_source_file(fn);
    }

    if (filename_implies_progname(fn))
    {
        nstring progname = progname_from_dir_of(fn);
        data.remember_progdir(fn.dirname());
        nstring bin_name = "bin/" + progname + data.exeext();
        data.remember_all_bin(bin_name);
        data.remember_clean_misc_file(".bin");
        data.remember_clean_misc_file("core");

        if (is_installable(progname))
        {
            nstring install_bin_name =
                (
                    "$(bindir)/"
                +
                    data.get_program_prefix()
                +
                    progname
                +
                    data.get_program_suffix()
                +
                    data.exeext()
                );
            data.remember_install_bin(install_bin_name);
        }
    }

    nstring dot_o_file = fn.trim_extension() + ".o";
    data.remember_clean_obj_file(dot_o_file);
    if (data.use_libtool())
    {
        nstring dot_lo_file = fn.trim_extension() + ".lo";
        data.remember_object_file(dot_lo_file);
        data.remember_clean_obj_file(dot_lo_file);
        data.remember_dist_clean_dir(fn.dirname() + "/.libs");
    }
    else
    {
        data.remember_object_file(dot_o_file);
    }
}


// vim: set ts=8 sw=4 et :
