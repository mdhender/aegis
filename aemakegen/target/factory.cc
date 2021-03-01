//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2010, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/fstrcmp.h>
#include <common/sizeof.h>
#include <libaegis/sub.h>

#include <aemakegen/target/debian.h>
#include <aemakegen/target/make/automake.h>
#include <aemakegen/target/make/makefile.h>
#include <aemakegen/target/pkg-config.h>
#include <aemakegen/target/rpm-spec.h>


struct table_t
{
    const char *name;
    target::pointer (*func)(change_identifier &);
};

static const table_t table[] =
{
    { "automake", target_make_automake::create },
    { "debian", target_debian::create },
    { "makefile", target_make_makefile::create },
    { "pkg-config", target_pkg_config::create },
    { "rpm-spec", target_rpm_spec::create },
};


target::pointer
target::create(const nstring &name, change_identifier &cid)
{
    if (name.empty())
        return target_make_makefile::create(cid);

    const table_t *best = 0;
    double best_weight = 0.6;
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (name == tp->name)
            return tp->func(cid);
        double w = fstrcmp(name.c_str(), tp->name);
        if (best_weight < w)
        {
            best_weight = w;
            best = tp;
        }
    }
    if (best)
    {
        sub_context_ty sc;
        sc.var_set_string("name", name);
        sc.var_set_string("guess", best->name);
        sc.fatal_intl(i18n("no \"$name\", guessing \"$guess\""));
    }
    else
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        sc.fatal_intl(i18n("the name \"$name\" is undefined"));
    }
    return target::pointer();
}


void
target::list(void)
{
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        printf("%s\n", tp->name);
    }
}


// vim: set ts=8 sw=4 et :
