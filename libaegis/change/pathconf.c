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
 * MANIFEST: functions to manipulate pathconfs
 */

#include <change.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>


int
change_pathconf_name_max(cp)
	change_ty	*cp;
{
	string_ty	*bl;
	int		bl_max;
	string_ty	*dd;
	int		dd_max;

	assert(cp->reference_count >= 1);
	bl = project_baseline_path_get(cp->pp, 0);
	dd = change_development_directory_get(cp, 0);
	change_become(cp);
	bl_max = os_pathconf_name_max(bl);
	dd_max = os_pathconf_name_max(dd);
	change_become_undo();
	return (bl_max < dd_max ? bl_max : dd_max);
}
