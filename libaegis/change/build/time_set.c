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
 * MANIFEST: functions to manipulate time_sets
 */

#include <change.h>
#include <change/architecture/find_variant.h>
#include <error.h> /* for assert */
#include <trace.h>


void
change_build_time_set(cp)
	change_ty	*cp;
{
	long		j, k;
	cstate_architecture_times tp;
	cstate		cstate_data;

	/*
	 * set the build_time in the architecture variant record
	 */
	trace(("change_build_time_set(cp = %8.8lX)\n{\n"/*}*/, (long)cp));
	assert(cp->reference_count >= 1);
	tp = change_find_architecture_variant(cp);
	time(&tp->build_time);
	tp->test_time = 0;
	tp->test_baseline_time = 0; /* XXX */
	tp->regression_test_time = 0;

	/*
	 * set the build_time in the change state.
	 * figure the oldest time of all variants.
	 * if one is missing, then is zero.
	 */
	cstate_data = change_cstate_get(cp);
	cstate_data->build_time = tp->build_time;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		for (k = 0; k < cstate_data->architecture_times->length; ++k)
		{
			tp = cstate_data->architecture_times->list[k];
			if
			(
				str_equal
				(
					cstate_data->architecture->list[j],
					tp->variant
				)
			)
				break;
		}
		if (k >= cstate_data->architecture_times->length)
		{
			cstate_data->build_time = 0;
			break;
		}
		if (tp->build_time < cstate_data->build_time)
			cstate_data->build_time = tp->build_time;
	}
	cstate_data->test_time = 0;
	cstate_data->test_baseline_time = 0; /* XXX */
	cstate_data->regression_test_time = 0;
	trace((/*{*/"}\n"));
}