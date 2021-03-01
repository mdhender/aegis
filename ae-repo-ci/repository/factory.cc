//
// aegis - project change supervisor
// Copyright (C) 2006-2008, 2012 Peter Miller
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/error.h>
#include <common/fstrcmp.h>
#include <common/sizeof.h>

#include <ae-repo-ci/repository/cvs.h>
#include <ae-repo-ci/repository/subversion.h>


static repository *
new_repository_cvs()
{
    return new repository_cvs();
}


static repository *
new_repository_subversion()
{
    return new repository_subversion();
}


struct table_t
{
    const char *type;
    repository *(*alloc)(void);
    const char *description;
    bool hide;
};


static const table_t table[] =
{
    { "cvs", new_repository_cvs, "Concurrent Version System", false },
    { "svn", new_repository_subversion, "Subversion", false },
    { "subversion", new_repository_subversion, "svn", true },
};


void
repository::list()
{
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        if (!tp->hide)
            printf("%.7s\t%s\n", tp->type, tp->description);
    }
}


repository *
repository::factory(const char *type)
{
    const table_t *best = 0;
    double best_weight = 0.6;
    for (const table_t *tp = table; tp < ENDOF(table); ++tp)
    {
        double w = fstrcmp(type, tp->type);
        if (w == 1)
            return tp->alloc();
        if (w > best_weight)
        {
            best = tp;
            best_weight = w;
        }
    }
    if (best)
    {
        fatal_raw
        (
            "repository type \"%s\" unknown, closest is \"%s\"",
            type,
            best->type
        );
    }
    fatal_raw("repository type \"%s\" unknown", type);
    return 0;
}


// vim: set ts=8 sw=4 et :
