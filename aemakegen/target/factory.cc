//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/fstrcmp.h>
#include <libaegis/sub.h>

#include <aemakegen/target/automake.h>
#include <aemakegen/target/makefile.h>


static target *
new_target_automake()
{
    return new target_automake();
}


static target *
new_target_makefile()
{
    return new target_makefile();
}


struct table_t
{
    const char *name;
    target *(*func)();
};

static const table_t table[] =
{
    { "makefile", new_target_makefile },
    { "automake", new_target_automake },
};


target *
target::create(const nstring &name)
{
    if (name.empty())
        return new target_makefile();
    const table_t *best = 0;
    double best_weight = 0.6;
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (name == tp->name)
            return tp->func();
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
    return 0;
}
