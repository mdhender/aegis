//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2009, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <common/error.h>
#include <libaegis/gonzo.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <common/trace.h>


void
project::list_inner(string_list_ty &wlr)
{
    trace(("project::list_inner(this = %p, wlr = %p)\n{\n",
        this, &wlr));

    //
    // check each change
    // add it to the list if it is a branch
    //
    change::pointer cp = this->change_get();
    long *lp = 0;
    size_t len = 0;
    change_branch_sub_branch_list_get(cp, &lp, &len);
    for (size_t k = 0; k < len; ++k)
    {
        long cn = lp[k];
        trace(("cn = %ld\n", cn));
        change::pointer cp2 = change_alloc(this, cn);
        change_bind_existing(cp2);
        // active only
        if (cp2->is_a_branch())
        {
            project *pp2 = bind_branch(cp2);
            pp2->list_inner(wlr);
            project_free(pp2);
        }
        else
            change_free(cp2);
    }

    //
    // Add the cannonical name of this project to the list.
    //
    // We add the canonical name last, because the branches are almost
    // always more interesting, and so they are placed first.
    //
    wlr.push_back(name_get());

    // do NOT free "lp"
    // do NOT free "cp"
    trace(("}\n"));
}


void
project_list_get(string_list_ty *wlp)
{
    //
    // get the top-level project list
    //
    trace(("project_list_get()\n{\n"));
    wlp->clear();
    string_list_ty toplevel;
    gonzo_project_list(&toplevel);

    // sort the list of names
    toplevel.sort();

    //
    // chase down each one, looking for branches
    //
    for (size_t j = 0; j < toplevel.nstrings; ++j)
    {
        trace(("j = %d/%d\n", (int)j, (int)toplevel.nstrings));
        string_ty *name = toplevel.string[j];
        trace(("name = \"%s\"\n", name->str_text));
        project *pp = project_alloc(name);
        pp->bind_existing();

        //
        // watch out for permissions
        // (returns errno of attempt to read project state)
        //
        int err = project_is_readable(pp);
        trace(("err = %d\n", err));

        //
        // Recurse into readable branch trees.
        //
        if (err == 0)
        {
            pp->list_inner(*wlp);
        }
        else
            wlp->push_back(project_name_get(pp).get_ref());
        project_free(pp);
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
