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

#include <common/trace.h>

#include <aemakegen/process_data.h>
#include <aemakegen/process_item/man_page.h>
#include <aemakegen/target.h>
#include <aemakegen/util.h>


process_item_man_page::~process_item_man_page()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


process_item_man_page::process_item_man_page(target &a_tgt) :
    process_item(a_tgt, &target::process_item_man_page)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


process_item_man_page::pointer
process_item_man_page::create(target &a_tgt)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new process_item_man_page(a_tgt));
}


bool
process_item_man_page::condition(const nstring &fn)
{
    trace(("process_item_man_page::condition(fn = %s)\n{\n",
        fn.quote_c().c_str()));
    bool result = looks_like_a_man_page(fn);
    trace(("return %d;\n}\n", result));
    return result;
}


void
process_item_man_page::preprocess(const nstring &fn)
{
    trace(("process_item_man::preprocess(fn = %s)\n{\n", fn.quote_c().c_str()));
    assert(condition(fn));

    data.set_install_data_macro();
    data.remember_man_sources(fn);
    data.set_need_groff(); // to build

    //
    // If the file contains include directives, we will need soelim(1) to
    // resolve the includes just before we install the file into $(mandir).
    //
    if (contains_dot_so_directive(fn))
        data.set_need_soelim();

    //
    // Build a local cat1 directory.  It is just for show,
    // we will not be installing anything from it.
    //
    {
        const char *p = strstr(fn.c_str(), "/man");
        nstring dst;
        if (p)
            dst = fn.substr(0, p - fn.c_str()) + "/cat" + nstring(p + 4);
        else
        {
            nstring src = extract_man_page_details(fn);
            assert(src.starts_with("man"));
            dst = "cat" + src.substr(3, src.size() - 3);
        }

        data.remember_all_doc(dst);
    }

    //
    // Do not install the man pages of programs that are not going
    // to be installed.
    //
    {
        nstring name = fn.basename();
        if
        (
            name.ends_with(".1")
        ||
            name.ends_with(".8")
        ||
            name.ends_with(".man")
        )
        {
            name = name.trim_extension();
            trace(("name = %s\n", name.quote_c().c_str()));
            if (!is_installable(name))
            {
                trace(("not going to be installed\n"));
                trace(("}\n"));
                return;
            }
        }
    }

    //
    // Build the path of the file to be installed.
    //
    nstring stem = extract_man_page_details(fn);
    trace(("stem = %s;\n", stem.quote_c().c_str()));
    nstring file2 = "$(mandir)/" + stem;
    trace(("file2 = %s;\n", file2.quote_c().c_str()));
    data.remember_install_mandir(file2);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
