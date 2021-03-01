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

#include <common/ac/stdio.h>

#include <common/fstrcmp.h>
#include <common/sizeof.h>
#include <libaegis/sub.h>

#include <aemakegen/flavour.h>
#include <aemakegen/flavour/aegis.h>
#include <aemakegen/flavour/generic.h>


typedef flavour::pointer (*func_ptr)(target &tgt);

struct table_t
{
    const char *name;
    func_ptr create;
};

static const table_t table[] =
{
    { "aegis", &flavour_aegis::create },
    { "generic", &flavour_generic::create },
};


flavour::pointer
flavour::factory(const nstring &name, target &tgt)
{
    //
    // default to the generic flavour
    //
    if (name.empty())
        return flavour_generic::create(tgt);

    //
    // look for an exact match
    //
    nstring lc_name = name.downcase();
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (nstring(tp->name) == lc_name)
        {
            return tp->create(tgt);
        }
    }

    //
    // look again, this time with fuzzy string comparisons,
    // to give a better error message
    //
    const table_t *best = 0;
    double best_weight = 0.6;
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        double w = fstrcmp(tp->name, lc_name.c_str());
        if (best_weight < w)
        {
            best = tp;
            best_weight = w;
        }
    }
    if (best)
    {
        sub_context_ty sc;
        sc.var_set_string("name", name);
        sc.var_set_string("guess", best->name);
        sc.fatal_intl(i18n("no \"$name\", guessing \"$guess\""));
        //NOTREACHED
    }
    else
    {
        sub_context_ty sc;
        sc.var_set_string("Name", name);
        sc.fatal_intl(i18n("the name \"$name\" is undefined"));
        //NOTREACHED
    }
    return pointer();
}


void
flavour::list(void)
{
    for (const table_t *tp; tp < ENDOF(table); ++tp)
    {
        printf("%s\n", tp->name);
    }
}


// vim: set ts=8 sw=4 et :
