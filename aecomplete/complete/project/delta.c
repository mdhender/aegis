/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate deltas
 */

#include <complete/private.h>
#include <complete/project/delta.h>
#include <project.h>
#include <project/history.h>
#include <shell.h>
#include <str_list.h>


typedef struct complete_project_delta_ty complete_project_delta_ty;
struct complete_project_delta_ty
{
    complete_ty     inherited;
    project_ty      *pp;
};


static void destructor _((complete_ty *));

static void
destructor(cp)
    complete_ty     *cp;
{
    complete_project_delta_ty *this;

    this = (complete_project_delta_ty *)cp;
    project_free(this->pp);
}


static void perform _((complete_ty *, shell_ty *));

static void
perform(cp, sh)
    complete_ty     *cp;
    shell_ty        *sh;
{
    complete_project_delta_ty *this;
    string_ty       *prefix;
    size_t          j;

    this = (complete_project_delta_ty *)cp;
    prefix = shell_prefix_get(sh);
    for (j = 0; ; ++j)
    {
	long            cn;
	long            dn;
	string_list_ty  name;
	size_t		k;
	string_ty       *s;

	if (!project_history_nth(this->pp, j, &cn, &dn, &name))
	    break;

	s = str_format("%ld", dn);
	if (str_leading_prefix(s, prefix))
	    shell_emit(sh, s);
	str_free(s);

	for (k = 0; k < name.nstrings; ++k)
	{
	    s = name.string[k];
	    if (str_leading_prefix(s, prefix))
		shell_emit(sh, s);
	}
	string_list_destructor(&name);
    }
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_project_delta_ty),
    "project delta",
};


complete_ty *
complete_project_delta(pp)
    project_ty      *pp;
{
    complete_ty     *result;
    complete_project_delta_ty *this;

    result = complete_new(&vtbl);
    this = (complete_project_delta_ty *)result;
    this->pp = pp;
    return result;
}
