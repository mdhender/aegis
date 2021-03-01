/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate lists
 */

#include <change.h>
#include <change_bran.h>
#include <error.h>
#include <gonzo.h>
#include <project.h>
#include <str_list.h>
#include <trace.h>


static void project_list_inner _((string_list_ty *, project_ty *));

static void
project_list_inner(wlp, pp)
	string_list_ty	*wlp;
	project_ty	*pp;
{
	change_ty	*cp;
	long		*lp;
	size_t		len;
	long		k;

	/*
	 * add the cannonical name of this project to the list
	 */
	string_list_append(wlp, project_name_get(pp));

	/*
	 * check each change
	 * add it to the list if it is a branch
	 */
	cp = project_change_get(pp);
	change_branch_sub_branch_list_get(cp, &lp, &len);
	for (k = 0; k < len; ++k)
	{
		long		cn;
		change_ty	*cp2;

		cn = lp[k];
		trace(("cn = %ld\n", cn));
		cp2 = change_alloc(pp, cn);
		change_bind_existing(cp2);
		/* active only */
		if (change_is_a_branch(cp2))
		{
			project_ty	*pp2;

			pp2 = project_bind_branch(pp, cp2);
			project_list_inner(wlp, pp2);
			project_free(pp2);
		}
		else
			change_free(cp2);
	}
	/* do NOT free ``lp'' */
	/* do NOT free ``cp'' */
}


void
project_list_get(wlp)
	string_list_ty	*wlp;
{
	size_t		j;
	string_list_ty	toplevel;

	/*
	 * get the top-level project list
	 */
	trace(("project_list_get()\n{\n"));
	string_list_constructor(wlp);
	gonzo_project_list(&toplevel);

	/*
	 * chase down each one, looking for branches
	 */
	for (j = 0; j < toplevel.nstrings; ++j)
	{
		string_ty	*name;
		project_ty	*pp;
		int		err;

		name = toplevel.string[j];
		trace(("name = \"%s\"\n", name->str_text));
		pp = project_alloc(name);
		project_bind_existing(pp);

		/*
		 * watch out for permissions
		 * (returns errno of attempt to read project state)
		 */
		err = project_is_readable(pp);

		/*
		 * Recurse into readable branch trees.
		 */
		if (!err)
			project_list_inner(wlp, pp);
		else
			string_list_append(wlp, project_name_get(pp));
		project_free(pp);
	}
	string_list_destructor(&toplevel);

	/*
	 * sort the list of names
	 * (C locale)
	 */
	string_list_sort(wlp);
	trace(("}\n"));
}
