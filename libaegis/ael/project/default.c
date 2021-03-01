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

#include <ael/project/default.h>
#include <ael/change/inappropriat.h>
#include <ael/project/inappropriat.h>
#include <trace.h>
#include <user.h>


void
list_default_project(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	/*
	 * check for silly arguments
	 */
	trace(("list_default_project()\n{\n"/*}*/));
	if (project_name)
		list_project_inappropriate();
	if (change_number)
		list_change_inappropriate();

	/*
	 * Find default project name;
	 * will generate fatal error if no default.
	 */
	project_name = user_default_project();

	/*
	 * print it out
	 */
	printf("%s\n", project_name->str_text);
	trace((/*{*/"}\n"));
}
