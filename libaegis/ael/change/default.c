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
 * MANIFEST: functions to manipulate defaults
 */

#include <ac/stdio.h>

#include <ael/change/default.h>
#include <ael/change/inappropriat.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <zero.h>


void
list_default_change(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	project_ty	*pp;
	user_ty		*up;

	/*
	 * check for silly arguments
	 */
	trace(("list_default_change()\n{\n"));
	if (change_number)
		list_change_inappropriate();

	/*
	 * resolve the project name
	 */
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * Find default change number;
	 * will generate fatal error if no default.
	 */
	change_number = user_default_change(up);

	/*
	 * print it out
	 */
	printf("%ld\n", magic_zero_decode(change_number));

	/*
	 * clean up and go home
	 */
	project_free(pp);
	user_free(up);
	trace(("}\n"));
}
