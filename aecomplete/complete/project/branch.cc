//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate branchs
//

#include <change.h>
#include <change/branch.h>
#include <complete/private.h>
#include <complete/project/branch.h>
#include <project.h>
#include <shell.h>


typedef struct complete_project_branch_ty complete_project_branch_ty;
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
    change_branch_sub_branch_list_get(project_change_get(pp), &list, &len);
    for (j = 0; j < len; ++j)
    {
	change_ty       *cp;
	project_ty      *bp;

	cp = change_alloc(pp, list[j]);
	change_bind_existing(cp);
	if (!all && !change_is_a_branch(cp))
	{
	    change_free(cp);
	    continue;
	}
	bp = project_bind_branch(pp, cp);
	hunt(bp, all, sh);
	project_free(bp);
    }
}


static void
perform(complete_ty *cp, shell_ty *sh)
{
    complete_project_branch_ty *this_thing;
    project_ty      *pp;

    //
    // Locate the trunk.
    //
    this_thing = (complete_project_branch_ty *)cp;
    pp = this_thing->pp;
    while (pp && pp->parent)
	pp = pp->parent;

    //
    // Scan all of the branches outwards from the trunk.
    //
    hunt(pp, this_thing->all, sh);
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