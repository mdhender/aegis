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
 * MANIFEST: functions to check filenames for trojan suspectedness
 */

#include <change.h>
#include <pconf.h>
#include <project.h>
#include <project/file/trojan.h>
#include <gmatch.h>
#include <str.h>


static pconf project_pconf_get _((project_ty *));

static pconf
project_pconf_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_pconf_get(cp, 0);
}


int
project_file_trojan_suspect(pp, fn)
	project_ty	*pp;
	string_ty	*fn;
{
	pconf		pconf_data;
	size_t		j;
	string_ty	*pattern;

	/*
	 * When we check for trojans, we use the *project* config file,
	 * just in case the config file in the change set is *lying*.
	 */
	pconf_data = project_pconf_get(pp);

	/*
	 * Check the filename against each of the patterns.
	 */
	if (!pconf_data->trojan_horse_suspect)
		return 0;
	for (j = 0; j < pconf_data->trojan_horse_suspect->length; ++j)
	{
		pattern = pconf_data->trojan_horse_suspect->list[j];
		if (gmatch(pattern->str_text, fn->str_text))
			return 1;
	}

	/*
	 * No patterns matched, it must be OK.
	 */
	return 0;
}
