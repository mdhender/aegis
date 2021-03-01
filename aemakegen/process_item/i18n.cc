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

#include <common/error.h>
#include <libaegis/locale_name.h>

#include <aemakegen/process_data.h>
#include <aemakegen/process_item/i18n.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_i18n::~process_item_i18n()
{
}


process_item_i18n::process_item_i18n(target &a_tgt) :
    process_item(a_tgt, &target::process_item_i18n)
{
}


process_item_i18n::pointer
process_item_i18n::create(target &a_tgt)
{
    return pointer(new process_item_i18n(a_tgt));
}


bool
process_item_i18n::condition(const nstring &filename)
{
    nstring_list components;
    components.split(filename, "/");
    return
        (
            components.size() >= 3
        &&
            components[components.size() - 2].upcase() == "LC_MESSAGES"
        &&
            components[components.size() - 1].ends_with(".po")
        );
}


void
process_item_i18n::preprocess(const nstring &filename)
{
    if (filename.ends_with("/common.po"))
    {
        // Do nothing directly,
        // it will be used indirectly by other rules.
        return;
    }

    assert(condition(filename));
    nstring mo = filename.trim_extension() + ".mo";
    data.remember_all_i18n(mo);

    //
    // We have to calculate what the installed destination files
    // look like.  It is very similar to the .mo file we just built,
    // but has to arrive in the correct place, using just the right
    // abount of the .mo file's path.
    //
    // something like "/usr/share/locale/ru/LC_MESSAGES/aegis.mo"
    //          NLSDIR ^^^^^^^^^^^^^^^^^
    //  ${datarootdir} ^^^^^^^^^^
    //
    nstring_list src_parts;
    src_parts.split(mo, "/");
    while (src_parts.size() > 3)
        src_parts.pop_front();
    if (!is_a_locale_name(src_parts[0]))
    {
        fatal_raw
        (
            "%s: does not appear to include a locale name as the "
                "third-last path component (%s)",
            filename.c_str(),
            src_parts[0].c_str()
        );
    }
    if (!data.have_nlsdir())
    {
        fatal_raw
        (
            "%s: the configure.ac file does not appear to contain a "
            "AC_SUBST(NLSDIR) defintion, and yet the project would "
            "appear to require it (NLSDIR=${datarootdir}/locale)",
            filename.c_str()
        );
    }
    nstring dst = "$(NLSDIR)/" + src_parts.unsplit("/");
    data.remember_install_i18n(dst);
    data.set_install_data_macro();
}


// vim: set ts=8 sw=4 et :
