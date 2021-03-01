//
// aegis - project change supervisor
// Copyright (C) 1999, 2001, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/project/aliases.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/gonzo.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>


void
list_project_aliases(change_identifier &cid, string_list_ty *)
{
    string_list_ty  name;
    output::pointer name_col;
    output::pointer desc_col;
    col::pointer colp;
    int             nprinted = 0;

    trace(("list_project_aliases()\n{\n"));
    if (cid.set())
        list_change_inappropriate();

    //
    // list the projects
    //
    gonzo_alias_list(&name);
    name.sort();

    //
    // Find the longest name, and round it up so that the column is
    // always one less than a multiple of eight (it makes for nice numbers
    // of tabs).
    //
    size_t longest = PROJECT_WIDTH;
    for (size_t k = 0; k < name.nstrings; ++k)
    {
        size_t x = name.string[k]->str_length;
        if (longest < x)
            longest = x;
    }
    longest |= 7;

    //
    // create the columns
    //
    colp = col::open((string_ty *)0);
    colp->title("List of Project Aliases", (char *)0);

    int left = 0;
    name_col = colp->create(0, longest, "Alias\n---------");
    left += longest + 1;

    if (!option_terse_get())
    {
        desc_col =
            colp->create(left, 0, "Project\n-----------");
    }

    //
    // list each alias
    //
    nprinted = 0;
    for (size_t j = 0; j < name.nstrings; ++j)
    {
        if (cid.project_set())
        {
            string_ty       *other;

            other = gonzo_alias_to_actual(name.string[j]);
            assert(other);
            if
            (
                other
            &&
                (
                    str_equal(other, cid.get_pp()->name_get())
                ||
                    str_equal(name.string[j], cid.get_pp()->name_get())
                )
            )
            {
                name_col->fputs(name.string[j]);
                if (desc_col)
                    desc_col->fputs(other);
                colp->eoln();
                ++nprinted;
            }
        }
        else
        {
            name_col->fputs(name.string[j]);

            if (desc_col)
            {
                string_ty       *other;

                other = gonzo_alias_to_actual(name.string[j]);
                assert(other);
                if (other)
                    desc_col->fputs(other);
            }
            colp->eoln();
            ++nprinted;
        }
    }
    if (option_verbose_get() && !nprinted)
    {
        output::pointer fp = colp->create(0, 0, (const char *)0);
        fp->fputs("No project aliases.\n");
        colp->eoln();
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
