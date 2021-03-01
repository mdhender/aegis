//
// aegis - project change supervisor
// Copyright (C) 2008, 2009, 2011, 2012 Peter Miller
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

#include <common/page.h>
#include <libaegis/change/functor/debchangelog.h>
#include <libaegis/change/functor/debcloses.h>
#include <libaegis/change/functor/date_version.h>
#include <libaegis/col/pretty.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/wide_output/to_narrow.h>
#include <libaegis/zero.h>

#include <aemakegen/target/debian.h>


change::pointer
version_to_change(project *trunk_pp, const nstring &version)
{
    if (version.empty())
        return change_copy(trunk_pp->change_get());
    nstring_list parts;
    parts.split(version, ".");
    project *pp = trunk_pp;
    for (size_t j = 0;;)
    {
        nstring n = parts[j++];
        if (n[0] == 'D')
        {
            assert(j == parts.size());
            long dn = n.substr(1, n.size() - 1).to_long();
            long cn = project_delta_number_to_change_number(pp, dn);
            change::pointer cp = change_alloc(pp, cn);
            change_bind_existing(cp);
            return cp;
        }
        if (n[0] == 'C')
        {
            assert(j == parts.size());
            long cn = n.substr(1, n.size() - 1).to_long();
            cn = magic_zero_encode(cn);
            change::pointer cp = change_alloc(pp, cn);
            change_bind_existing(cp);
            return cp;
        }

        assert(j < parts.size());
        long cn = n.to_long();
        cn = magic_zero_encode(cn);
        change::pointer bp = change_alloc(pp, cn);
        change_bind_existing(bp);
        pp = pp->bind_branch(bp);
    }
}


void
target_debian::gen_changelog(void)
{
    nstring_list changes;
    change_functor_date_version func(changes);
    project_inventory_walk(get_pp(), func);

    // this sorts them by date
    changes.sort();

    os_become_orig();
    output::pointer fp = output_file::open("debian/changelog");
    os_become_undo();

    option_page_headers_set(false, 0);
    page_width_set(80);
    wide_output::pointer wop = wide_output_to_narrow::open(fp);

    change_functor_debcloses fc;
    project_inventory_walk(get_pp(), fc);

    col::pointer colp = col_pretty::create(wop);
    change_functor_debchangelog f2(colp);
    f2.print(get_cp(), fc.get_result());
    project *tpp = get_pp()->trunk_get();
    for (size_t j = 0; j < changes.size(); ++j)
    {
        nstring v1 = changes[changes.size() - 1 - j];
        const char *p = strchr(v1.c_str(), '/');
        assert(p);
        if (!p)
            continue;
        nstring v(p + 1);
        change::pointer cp = version_to_change(tpp, v);
        f2(cp);
        change_free(cp);
    }
}


// vim: set ts=8 sw=4 et :
