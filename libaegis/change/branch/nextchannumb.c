/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate nextchannumbs
 */

#include <change/branch.h>
#include <error.h> /* for assert */
#include <interval.h>
#include <skip_unlucky.h>
#include <trace.h>


long
change_branch_next_change_number(change_ty *cp, int is_a_change)
{
	cstate		cstate_data;
	cstate_branch_change_list lp;
	long		change_number;
	interval_ty	*ip1;
	interval_ty	*ip2;
	interval_ty	*ip3;
	size_t		j, k;
	long		min;
	int		reuse;

	trace(("change_branch_next_change_number(cp = %8.8lX)\n{\n"/*}*/,
		(long)cp));
	cstate_data = change_cstate_get(cp);
	assert(cstate_data->branch);
	if (!cstate_data->branch->change)
		cstate_data->branch->change =
			cstate_branch_change_list_type.alloc();
	lp = cstate_data->branch->change;

	/*
	 * build an interval which contains the set of changes
	 * (it could be empty)
	 */
	ip1 = interval_create_empty();
	for (j = 0; j < lp->length; )
	{
		for (k = j + 1; k < lp->length; ++k)
			if (lp->list[k - 1] + 1 != lp->list[k])
				break;
		ip2 = interval_create_range(lp->list[j], lp->list[k - 1]);
		ip3 = interval_union(ip1, ip2);
		interval_free(ip1);
		interval_free(ip2);
		ip1 = ip3;
		j = k;
	}

	/*
	 * difference the above set from the interval [min..max)
	 * This avoids zero.  If the user wants a zero-numbered change,
	 * she must ask for it.
	 */
	if (is_a_change)
		min = change_branch_minimum_change_number_get(cp);
	else
		min = change_branch_minimum_branch_number_get(cp);
	ip2 = interval_create_range(min, TRUNK_CHANGE_NUMBER - 1);
	ip3 = interval_difference(ip2, ip1);
	interval_free(ip1);
	interval_free(ip2);
	assert(ip3->length);

	/*
	 * The result is the minimum acceptable number in the interval.
	 * But !reuse means the first number in the last interval.
	 */
	reuse = !is_a_change || change_branch_reuse_change_numbers_get(cp);
	if (!change_branch_skip_unlucky_get(cp))
	{
		if (reuse)
			change_number = ip3->data[0];
		else
			change_number = ip3->data[ip3->length - 2];
	}
	else
	{
		for (;;)
		{
			if (reuse)
				change_number = ip3->data[0];
			else
				change_number = ip3->data[ip3->length - 2];
			min = skip_unlucky(change_number);
			if (min == change_number)
				break;
			ip1 = interval_create_range(change_number, min - 1);
			ip2 = interval_difference(ip3, ip1);
			interval_free(ip3);
			interval_free(ip1);
			ip3 = ip2;
		}
	}
	interval_free(ip3);
	trace(("return %ld;\n", change_number));
	trace((/*{*/"}\n"));
	return change_number;
}
