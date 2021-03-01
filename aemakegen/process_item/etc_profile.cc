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
#include <common/ac/string.h>

#include <aemakegen/process_data.h>
#include <aemakegen/process_item/etc_profile.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_etc_profile::~process_item_etc_profile()
{
}


process_item_etc_profile::process_item_etc_profile(target &a_tgt) :
    process_item(a_tgt, &target::process_item_etc_profile)
{
}


process_item_etc_profile::pointer
process_item_etc_profile::create(target &a_tgt)
{
    return pointer(new process_item_etc_profile(a_tgt));
}


bool
process_item_etc_profile::condition(const nstring &fn)
{
    if (!fn.starts_with("lib/"))
        return false;
    nstring base = fn.basename();
    if (base.starts_with("profile."))
        return true;
    return (base == "cshrc" || base == "profile");
}


void
process_item_etc_profile::preprocess(const nstring &fn)
{
    assert(condition(fn));
    data.set_seen_datadir();
    data.set_seen_sysconfdir();
    data.set_install_script_macro();

    nstring rest(fn.trim_first_directory());
    nstring dir(fn.dirname());

    {
        nstring dst = "$(datadir)/" + get_project_name() + "/" + rest;
        data.remember_install_datadir(dst);
    }

    {
        nstring dst = "$(sysconfdir)/profile.d/" + get_project_name() + ".";
        nstring ext = fn.get_extension();
        if (ext.empty())
            dst += (strstr(fn.c_str(), "csh") ? "csh" : "sh");
        else
            dst += ext;

        // not quite the right file list, but what the heck
        data.remember_install_datadir(dst);
    }
}


// vim: set ts=8 sw=4 et :
