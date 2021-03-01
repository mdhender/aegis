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

#include <libaegis/change.h>
#include <libaegis/change/architecture/find_variant.h>
#include <common/error.h> // for assert
#include <libaegis/change/file.h>


//
// NAME
//	change_test_times_clear
//
// SYNOPSIS
//	void change_test_times_clear(change::pointer );
//
// DESCRIPTION
//	The change_test_times_clear function is called by aeb to reset
//	the test times after a build.
//

void
change_test_times_clear(change::pointer cp)
{
	cstate_architecture_times_ty *tp;
	cstate_ty       *cstate_data;
	size_t		j;
	string_ty	*variant;

	//
	// reset the test times in the architecture variant record
	//
	assert(cp->reference_count >= 1);
	tp = change_find_architecture_variant(cp);
	tp->test_time = 0;
	tp->test_baseline_time = 0;
	tp->regression_test_time = 0;

	//
	// reset the test times in the change state.
	//
	cstate_data = cp->cstate_get();
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;

	//
	// reset file test times
	//
	variant = change_architecture_name(cp, 1);
	for (j = 0; ; ++j)
	{
		fstate_src_ty   *src_data;
		fstate_src_architecture_times_list_ty *atlp;
		size_t		k;

		src_data = change_file_nth(cp, j, view_path_first);
		if (!src_data)
			break;
		atlp = src_data->architecture_times;
		if (!atlp)
			continue;

		//
		// only reset the relevant architecture
		//
		for (k = 0; k < atlp->length; ++k)
		{
			fstate_src_architecture_times_ty *atp;

			atp = atlp->list[k];
			if
			(
				// bug if not set
				atp->variant
			&&
				str_equal(atp->variant, variant)
			)
			{
				atp->test_time = 0;
				atp->test_baseline_time = 0;
				break;
			}
		}
	}
}
