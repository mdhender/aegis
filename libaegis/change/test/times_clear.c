/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate times_clears
 */

#include <change.h>
#include <change/architecture/find_variant.h>
#include <error.h> /* for assert */
#include <change/file.h>


/*
 * NAME
 *	change_test_times_clear
 *
 * SYNOPSIS
 *	void change_test_times_clear(change_ty *);
 *
 * DESCRIPTION
 *	The change_test_times_clear function is called by aeb to reset
 *	the test times after a build.
 */

void
change_test_times_clear(change_ty *cp)
{
	cstate_architecture_times_ty *tp;
	cstate_ty       *cstate_data;
	size_t		j;
	string_ty	*variant;

	/*
	 * reset the test times in the architecture variant record
	 */
	assert(cp->reference_count >= 1);
	tp = change_find_architecture_variant(cp);
	tp->test_time = 0;
	tp->test_baseline_time = 0;
	tp->regression_test_time = 0;

	/*
	 * reset the test times in the change state.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0;
	cstate_data->regression_test_time = 0;

	/*
	 * reset file test times
	 */
	variant = change_architecture_name(cp, 1);
	for (j = 0; ; ++j)
	{
		fstate_src_ty   *src_data;
		fstate_src_architecture_times_list_ty *atlp;
		size_t		k;

		src_data = change_file_nth(cp, j);
		if (!src_data)
			break;
		atlp = src_data->architecture_times;
		if (!atlp)
			continue;

		/*
		 * only reset the relevant architecture
		 */
		for (k = 0; k < atlp->length; ++k)
		{
			fstate_src_architecture_times_ty *atp;

			atp = atlp->list[k];
			if
			(
				/* bug if not set */
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
