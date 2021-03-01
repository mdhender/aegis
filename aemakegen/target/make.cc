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

#include <common/trace.h>

#include <aemakegen/target/make.h>
#include <aemakegen/util.h>


target_make::~target_make()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


target_make::target_make(change_identifier &a_cid) :
    target(a_cid)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
target_make::process_item_aegis_fmtgen(const nstring &fn)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    nstring stem = fn.trim_extension();
    nstring root(fn.basename());
    nstring dir(fn.dirname());
    nstring oc = stem + ".fmtgen.cc";
    nstring oh = stem + ".fmtgen.h";

    {
        nstring exe = "bin/fmtgen" + data.exeext();

        location_comment(__FILE__, __LINE__);
        nstring_list lhs;
        lhs.push_back(oc);
        lhs.push_back(oh);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back(exe);
        rhs.push_back_unique(c_include_dependencies(fn));
        nstring_list body;
        body.push_back(exe + " -I. " + fn + " " + oc + " " + oh);
        print_rule(lhs, rhs, body);
    }

    // and also the output
    processing.run_process(oc);
    processing.run_process(oh);
    trace(("}\n"));
}


void
target_make::process_item_configure_ac_in(const nstring &fn)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    nstring fn2 = nstring(fn.c_str(), fn.size() - 3);

    // this is an automake file that needs to pass through
    // config.status (written by ./configure scipt) in order to produce
    // system-specific customised content.
    {
        nstring_list lhs;
        lhs.push_back(fn2);
        nstring_list rhs;
        rhs.push_back(fn);
        nstring_list body;

        nstring package_name = get_project_name();
        nstring package_name_subst =
            "s|@PACKAGE_NAME@|" + package_name.quote_c() + "|g";
        nstring package_version = get_project_version();
        nstring package_version_subst =
            "s|@PACKAGE_VERSION@|" + package_version.quote_c() + "|g";
        nstring command =
            "sed "
            "-e " + package_name_subst.quote_shell() + " "
            "-e " + package_version_subst.quote_shell() + " " +
            fn + " "
            "> $@"
            ;
        body.push_back(command);
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    // now process the result of the rule.
    // it could be just about anything.
    processing.run_process(fn2);
    trace(("}\n"));
}


void
target_make::process_item_autoconf(const nstring &fn)
{
    trace(("%s\n{\n", __PRETTY_FUNCTION__));
    nstring fn2 = nstring(fn.c_str(), fn.size() - 3);

    // this is an automake file that needs to pass through
    // config.status (written by ./configure scipt) in order to produce
    // system-specific customised content.
    {
        nstring_list lhs;
        lhs.push_back(fn2);
        nstring_list rhs;
        rhs.push_back(fn);
        rhs.push_back("config.status");
        nstring_list body;
        nstring cf = "CONFIG_FILES=";
        nstring ch = "CONFIG_HEADERS=";
        if (is_an_include_file(fn2))
            ch += "$@:" + fn;
        else
            cf += "$@:" + fn;
        body.push_back(cf + " " + ch + " " + "$(SH) config.status");
        location_comment(__FILE__, __LINE__);
        print_rule(lhs, rhs, body);
    }

    // now process the result of the rule.
    // it could be just about anything.
    processing.run_process(fn2);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
