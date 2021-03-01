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

#include <common/error.h>
#include <common/nstring/list.h>
#include <common/sizeof.h>

#include <aemakegen/util.h>


bool
is_a_source_file(const nstring &filename)
{
    nstring s = filename.basename().downcase();
    return
        (
            s.ends_with(".c")
        ||
            s.ends_with(".c++")
        ||
            s.ends_with(".cc")
        ||
            s.ends_with(".cpp")
        ||
            s.ends_with(".cxx")
        );
}


bool
is_an_include_file(const nstring &filename)
{
    nstring s = filename.basename().downcase();
    return
        (
            s.ends_with(".h")
        ||
            s.ends_with(".h++")
        ||
            s.ends_with(".hh")
        ||
            s.ends_with(".hpp")
        ||
            s.ends_with(".hxx")
        );
}


bool
is_a_c_source_file(const nstring &filename)
{
    return filename.ends_with(".c");
}


bool
filename_implies_progname(const nstring &filename)
{
    nstring_list components;
    components.split(filename, "/");
#if 0
    // some people prefer this pattern.
    // for example "fred/fred.c"
    if
    (
        components.size() == 2
    &&
        components[1].trim_extension() == components[0]
    )
    {
        return true;
    }
#endif
    return
        (
            components.size() >= 2
        &&
            components.back().trim_extension() == "main"
        );
}


nstring
progname_from_dir_of(const nstring &filename)
{
    // So it turns out I'm inconsistent.
    // The minus/hyphen is easier to type for real human commands,
    // the underscore is what I've historically used for tests.
    const char *repl = filename.starts_with("test/") ? "_" : "-";

    return filename.dirname().replace("/", repl);
}


bool
is_a_cxx_source_file(const nstring &filename)
{
    if (filename.ends_with(".C"))
        return true;
    nstring s = filename.basename().downcase();
    return
        (
            s.ends_with(".c++")
        ||
            s.ends_with(".cc")
        ||
            s.ends_with(".cpp")
        ||
            s.ends_with(".cxx")
        );
}


bool
extension_implies_script(const nstring &filename)
{
    nstring fn = filename;
    if (fn.ends_with(".in"))
        fn = fn.trim_extension();
    nstring ext = fn.get_extension();

    static const char *table[] =
    {
        "awk",
        "pl",
        "py",
        "sed",
        "sh",
        "tcl",
    };

    for (const char **tp = table; tp < ENDOF(table); ++tp)
    {
        if (nstring(*tp) == ext)
            return true;
    }
    return false;
}


// vim: set ts=8 sw=4 et :
