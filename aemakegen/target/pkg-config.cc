//
// aegis - project change supervisor
// Copyright (C) 2008, 2010-2012 Peter Miller
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
#include <common/ac/ctype.h>

#include <libaegis/project.h>

#include <aemakegen/target/pkg-config.h>
#include <aemakegen/util.h>


target_pkg_config::~target_pkg_config()
{
}


target_pkg_config::target_pkg_config(change_identifier &a_cid) :
    target(a_cid)
{
}


target::pointer
target_pkg_config::create(change_identifier &a_cid)
{
    return pointer(new target_pkg_config(a_cid));
}


static nstring
first_line_only(const nstring &s)
{
    unsigned max = 67;
    const char *start = s.c_str();
    const char *end = start + s.size();
    if (s.size() > max)
    end = start + max;
    const char *cp = start;
    while (cp < end)
    {
        unsigned char c = *cp;
        // tabs are OK, newlines are not
        if (c == '\t' || isprint(c))
            ++cp;
        else
            break;
    }
    return nstring(start, cp - start);
}


void
target_pkg_config::process3_end(void)
{
    op->fprintf("prefix=@prefix@\n");
    op->fprintf("exec_prefix=@exec_prefix@\n");
    op->fprintf("bindir=@bindir@\n");
    op->fprintf("libdir=@libdir@\n");
    if (data.seen_install_mandir())
        op->fprintf("mandir=@mandir@\n");
    if (data.seen_datadir())
        op->fprintf("datadir=@datadir@\n");
    op->fprintf("datarootdir=@datarootdir@\n");
    if (data.seen_install_include())
        op->fprintf("includedir=@includedir@\n");
    op->fprintf("\n");

    nstring source_name = get_project_name();
    op->fprintf("Name: %s\n", source_name.c_str());

    nstring source_description =
        nstring(project_brief_description_get(get_pp()->trunk_get()));
    source_description = first_line_only(source_description).trim();
    op->fprintf("Description: %s\n", source_description.c_str());

    op->fprintf("Version: %s\n", get_cp()->version_debian_get().c_str());

    nstring homepage =
        get_cp()->pconf_attributes_find("aemakegen:debian:homepage");
    if (!homepage.empty())
        op->fprintf("URL: %s\n", homepage.c_str());

    nstring requires =
        get_cp()->pconf_attributes_find("aemakegen:pkg-config:requires");
    requires = requires.trim();
    if (!requires.empty())
        op->fprintf("Requires: %s\n", requires.c_str());

    nstring conflicts =
        get_cp()->pconf_attributes_find("aemakegen:pkg-config:conflicts");
    conflicts = conflicts.trim();
    if (!conflicts.empty())
        op->fprintf("Conflicts: %s\n", conflicts.c_str());

    nstring libs = get_cp()->pconf_attributes_find("aemakegen:pkg-config:libs");
    nstring libname = get_library_libname();
    assert(libname.starts_with("lib"));
    libs = "-l" + libname.substr(3, 999)  + " " + libs;
    libs = "-L${libdir} " + libs;
    libs = libs.trim();
    op->fprintf("Libs: %s\n", libs.c_str());

    nstring libs_private =
        get_cp()->pconf_attributes_find("aemakegen:pkg-config:libs.private");
    libs_private = "@LIBS@ " + libs_private;
    libs_private = libs_private.trim();
    op->fprintf("Libs.private: %s\n", libs_private.c_str());

    nstring cflags =
        get_cp()->pconf_attributes_find("aemakegen:pkg-config:cflags");
    if (data.seen_install_include())
        cflags = "-I${includedir} " + cflags;
    cflags = cflags.trim();
    if (!cflags.empty())
        op->fprintf("Cflags: %s\n", cflags.c_str());
}


// vim: set ts=8 sw=4 et :
