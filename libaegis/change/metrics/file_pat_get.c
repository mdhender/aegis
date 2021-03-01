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
 * MANIFEST: functions to manipulate file_pat_gets
 */

#include <change.h>
#include <error.h> /* for assert */


string_ty *
change_metrics_filename_pattern_get(cp)
	change_ty	*cp;
{
	pconf		pconf_data;

	/*
	 * Get the project config file.  Don't insist that it exists,
	 * because it will not for the first change.
	 */
	assert(cp->reference_count >= 1);
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->metrics_filename_pattern)
		pconf_data->metrics_filename_pattern =
			str_from_c("$filename,S");
	return pconf_data->metrics_filename_pattern;
}
