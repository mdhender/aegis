//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate lists
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
    //
    // add the cannonical name of this project to the list
    //
    wlr.push_back(name_get());

    //
    // check each change
    // add it to the list if it is a branch
    //
    change_ty *cp = this->change_get();
    long *lp = 0;
    size_t len = 0;
    change_branch_sub_branch_list_get(cp, &lp, &len);
    for (size_t k = 0; k < len; ++k)
    {
	long cn = lp[k];
	trace(("cn = %ld\n", cn));
	change_ty *cp2 = change_alloc(this, cn);
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

    //
    // chase down each one, looking for branches
    //
    for (size_t j = 0; j < toplevel.nstrings; ++j)
    {
	string_ty *name = toplevel.string[j];
	trace(("name = \"%s\"\n", name->str_text));
	project_ty *pp = project_alloc(name);
	pp->bind_existing();

	//
	// watch out for permissions
	// (returns errno of attempt to read project state)
	//
	int err = project_is_readable(pp);

	//
	// Recurse into readable branch trees.
	//
	if (!err)
	    pp->list_inner(*wlp);
	else
	    wlp->push_back(project_name_get(pp));
	project_free(pp);
    }

    //
    // sort the list of names
    // (C locale)
    //
    // Project names look a lot like versions strings (indeed,
    // the tail ends *are* version strings) so sort them as such.
    //
    wlp->sort_version();
    trace(("}\n"));
}
