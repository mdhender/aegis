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
 * MANIFEST: functions to manipulate list_checks
 */

#include <change/file.h>


void
change_file_list_metrics_check(cp)
	change_ty	*cp;
{
	size_t		j;
	fstate_src	src_data;

	for (j = 0; ; ++j)
	{
		metric_list	mlp;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;

		/*
		 * Only verify the metrics for primary source files,
		 * and only for creates and modifies.
		 */
		if
		(
			src_data->usage == file_usage_build
		||
			(
				src_data->action != file_action_create
			&&
				src_data->action != file_action_modify
			)
		)
			continue;

		/*
		 * Read the file.
		 * The contents will be checked.
		 */
		mlp = change_file_metrics_get(cp, src_data->file_name);

		/*
		 * Throw the data away,
		 * we only wanted the checking side-effect.
		 */
		metric_list_type.free(mlp);
	}
}
