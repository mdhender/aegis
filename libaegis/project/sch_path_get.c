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
 * MANIFEST: functions to manipulate sch_path_gets
 */

#include <project/file.h>
#include <str_list.h>


void
project_search_path_get(pp, wlp, resolve)
	project_ty	*pp;
	string_list_ty		*wlp;
	int		resolve;
{
	project_ty	*ppp;

	/*
	 * do NOT call change_search_path, it will make a mess
	 */
	for (ppp = pp; ppp; ppp = ppp->parent)
		string_list_append(wlp, project_baseline_path_get(ppp, resolve));
}
