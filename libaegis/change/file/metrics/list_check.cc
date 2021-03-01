//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change/file.h>


static int
candidate(fstate_src_ty *src)
{
    switch (src->usage)
    {
    case file_usage_source:
    case file_usage_config:
    case file_usage_test:
    case file_usage_manual_test:
	switch (src->action)
	{
	case file_action_create:
	case file_action_modify:
	    return 1;

	case file_action_remove:
	case file_action_transparent:
	case file_action_insulate:
	    break;
	}
	break;

    case file_usage_build:
	break;
    }
    return 0;
}


void
change_file_list_metrics_check(change::pointer cp)
{
    size_t          j;
    fstate_src_ty   *src_data;

    for (j = 0; ; ++j)
    {
	metric_list_ty  *mlp;

	src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;

	//
	// Only verify the metrics for primary source files,
	// and only for creates and modifies.
	//
	if (candidate(src_data))
	    continue;

	//
	// Read the file.
	// The contents will be checked.
	//
	mlp = change_file_metrics_get(cp, src_data->file_name);

	//
	// Throw the data away,
	// we only wanted the checking side-effect.
	//
	metric_list_type.free(mlp);
    }
}
