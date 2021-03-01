//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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
#include <aecomplete/complete/private.h>
#include <aecomplete/complete/project/branch.h>
#include <libaegis/project.h>
#include <aecomplete/shell.h>


struct complete_project_branch_ty
{
    complete_ty     inherited;
    project_ty      *pp;
    int             all;
};


static void
destructor(complete_ty *cp)
{
    complete_project_branch_ty *this_thing;

    this_thing = (complete_project_branch_ty *)cp;
    project_free(this_thing->pp);
}


static void
hunt(project_ty *pp, int all, shell_ty *sh)
{
    string_ty       *prefix;
    string_ty       *s;
    long            *list;
    size_t          len;
    size_t          j;

    //
    // See if the project's short version matches.
    //
    prefix = shell_prefix_get(sh);
    s = project_version_short_get(pp);
    if (!s->str_length)
    {
	str_free(s);
	s = str_from_c("-");
    }
    if (str_leading_prefix(s, prefix))
	shell_emit(sh, s);
    str_free(s);

    //
    // Now hunt the sub-branches.
    //
    change_branch_sub_branch_list_get(pp->change_get(), &list, &len);
    for (j = 0; j < len; ++j)
    {
	change::pointer cp;
	project_ty      *bp;

	cp = change_alloc(pp, list[j]);
	change_bind_existing(cp);
	if (!all && !change_is_a_branch(cp))
	{
	    change_free(cp);
	    continue;
	}
	bp = pp->bind_branch(cp);
	hunt(bp, all, sh);
	project_free(bp);
    }
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    //
    // Locate the trunk.
    //
    complete_project_branch_ty *this_thing = (complete_project_branch_ty *)cp;
    project_ty *pp = this_thing->pp;

    //
    // Scan all of the branches outwards from the trunk.
    //
    hunt(pp->trunk_get(), this_thing->all, sh);
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_project_branch_ty),
    "project branch",
};


complete_ty *
complete_project_branch(project_ty *pp, int all)
{
    complete_ty     *result;
    complete_project_branch_ty *this_thing;

    result = complete_new(&vtbl);
    this_thing = (complete_project_branch_ty *)result;
    this_thing->pp = pp;
    this_thing->all = !!all;
    return result;
}
