/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to implement the builtin now function
 */

#include <aer/expr.h>
#include <aer/func/now.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/time.h>
#include <error.h>
#include <trace.h>


static int now_verify _((rpt_expr_ty *));

static int
now_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 0);
}


static rpt_value_ty *now_run _((rpt_expr_ty *, size_t, rpt_value_ty **));

static rpt_value_ty *
now_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	time_t		t;

	assert(argc == 0);
	time(&t);
	return rpt_value_time(t);
}


rpt_func_ty rpt_func_now =
{
	"now",
	0, /* optimizable */
	now_verify,
	now_run,
};


#define HOURS_PER_WORKING_DAY 7.5
#define SECONDS_PER_WORKING_DAY (long)(HOURS_PER_WORKING_DAY * 60L * 60L)
#define SECONDS_PER_DAY (24L * 60L * 60L)


double
working_days(start, finish)
	time_t		start;
	time_t		finish;
{
	time_t		duration;
	double		working_days_frac;
	long		working_days;
	long		ndays;
	long		nweeks;
	struct tm 	*tm;

	/*
	 * elapsed time in seconds
	 */
	trace(("working_days(start = %ld, finish = %ld)\n{\n"/*}*/, start,
		finish));
	trace(("start = %s", ctime(&start)));
	trace(("finish = %s", ctime(&finish)));
	duration = finish - start;
	if (duration < 0)
		duration = 0;
	trace(("duration = %ld;\n", duration));

	/*
	 * determine the number of whole calendar days
	 */
	ndays =
		(
			(duration + (SECONDS_PER_DAY - SECONDS_PER_WORKING_DAY))
		/
			SECONDS_PER_DAY
		);
	trace(("ndays = %ld;\n", ndays));

	/*
	 * determine the fractional part
	 */
	working_days_frac =
		(
			(double)(duration - ndays * SECONDS_PER_DAY)
		/
			SECONDS_PER_WORKING_DAY
		);
	trace(("working_days_frac = %g;\n", working_days_frac));

	/*
	 * 5 working days per working week
	 */
	nweeks = ndays / 7;
	working_days = 5 * nweeks;
	ndays -= nweeks * 7;
	working_days += ndays;
	trace(("working_days = %ld;\n", working_days));

	/*
	 * give credit for working over the weekend
	 */
	tm = localtime(&start);
	ndays += tm->tm_wday;
	trace(("ndays = %ld;\n", ndays));
	if (ndays >= 7 && tm->tm_wday != 6)
		working_days--;
	if (ndays >= 8)
		working_days--;
	if (working_days < 0)
		working_days = 0;
	trace(("working_days = %ld;\n", working_days));

	/*
	 * done
	 */
	working_days_frac += working_days;
	if (working_days_frac < 0)
		working_days_frac = 0;
	trace(("return %.10g;\n", working_days_frac));
	trace((/*{*/"}\n"));
	return working_days_frac;
}


static int working_days_verify _((rpt_expr_ty *));

static int
working_days_verify(ep)
	rpt_expr_ty	*ep;
{
	return (ep->nchild == 2);
}


static rpt_value_ty *working_days_run _((rpt_expr_ty *, size_t,
	rpt_value_ty **));

static rpt_value_ty *
working_days_run(ep, argc, argv)
	rpt_expr_ty	*ep;
	size_t		argc;
	rpt_value_ty	**argv;
{
	rpt_value_ty	*t1;
	rpt_value_ty	*t2;
	rpt_value_ty	*result;

	assert(argc == 2);
	t1 = rpt_value_integerize(argv[0]);
	if (t1->method->type != rpt_value_type_integer)
	{
		rpt_value_free(t1);
		result =
			rpt_value_error
			(
		 "working_days: argument 1: time value required (was given %s)",
				argv[0]->method->name
			);
		rpt_expr_error(ep, "%S", rpt_value_error_query(result));
		return result;
	}

	t2 = rpt_value_integerize(argv[1]);
	if (t2->method->type != rpt_value_type_integer)
	{
		rpt_value_free(t1);
		rpt_value_free(t2);
		result =
			rpt_value_error
			(
		 "working_days: argument 2: time value required (was given %s)",
				argv[1]->method->name
			);
		rpt_expr_error(ep, "%S", rpt_value_error_query(result));
		return result;
	}

	result =
		rpt_value_real
		(
			working_days
			(
				rpt_value_integer_query(t1),
				rpt_value_integer_query(t2)
			)
		);
	return result;
}


rpt_func_ty rpt_func_working_days =
{
	"working_days",
	1, /* optimizable */
	working_days_verify,
	working_days_run,
};
