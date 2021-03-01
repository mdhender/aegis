//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <common/error.h>
#include <libaegis/gonzo.h>
#include <libaegis/project.h>
#include <common/str_list.h>
#include <common/trace.h>


void
project_ty::list_inner(string_list_ty &wlr)
{
    trace(("project_ty::list_inner(this = %08lX, wlr = %08lX)\n{\n",
        (long)this, (long)&wlr));

    //
    // add the cannonical name of this project to the list
    //
    wlr.push_back(name_get());

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
	if (change_is_a_branch(cp2))
	{
	    project_ty *pp2 = bind_branch(cp2);
	    pp2->list_inner(wlr);
	    project_free(pp2);
	}
	else
	    change_free(cp2);
    }
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
    trace(("mark\n"));

    //
    // chase down each one, looking for branches
    //
    for (size_t j = 0; j < toplevel.nstrings; ++j)
    {
        trace(("j = %d/%d\n", (int)j, (int)toplevel.nstrings));
	string_ty *name = toplevel.string[j];
	trace(("name = \"%s\"\n", name->str_text));
	project_ty *pp = project_alloc(name);
        trace(("mark\n"));
	pp->bind_existing();
        trace(("mark\n"));

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
            trace(("libaegis/project/list.cc\n"));
	    pp->list_inner(*wlp);
            trace(("libaegis/project/list.cc\n"));
        }
	else
	    wlp->push_back(project_name_get(pp));
        trace(("libaegis/project/list.cc\n"));
	project_free(pp);
        trace(("libaegis/project/list.cc\n"));
    }

    //
    // sort the list of names
    // (C locale)
    //
    // Project names look a lot like versions strings (indeed,
    // the tail ends *are* version strings) so sort them as such.
    //
    trace(("mark\n"));
    wlp->sort_version();
    trace(("}\n"));
}
