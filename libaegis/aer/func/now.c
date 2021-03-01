/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to implement the builtin now function
 */

#include <aer/expr.h>
#include <aer/func/now.h>
#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/time.h>
#include <error.h>
#include <sub.h>
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


#define WORKING_DAYS_PER_WEEK 5
#define HOURS_PER_WORKING_DAY 7.5
#define SECONDS_PER_WORKING_DAY (long)(HOURS_PER_WORKING_DAY * 60L * 60L)
#define SECONDS_PER_DAY (24L * 60L * 60L)


double
working_days(start, finish)
	time_t		start;
	time_t		finish;
{
	long		working_days_whole;
	double		working_days_frac;
	int		wday;

	/*
	 * Flip it end-for-end if they gave it the wrong way round.
	 */
	trace(("working_days(start = %ld, finish = %ld)\n{\n"/*}*/, start,
		finish));
	trace(("start = %s", ctime(&start)));
	trace(("finish = %s", ctime(&finish)));
	if (start > finish)
	{
		time_t		swap;

		swap = start;
		finish = start;
		start = swap;
	}

	/*
	 * Get the current week say.
	 * Adjust it so that MON=0 thru SUN=6
	 */
	wday = localtime(&start)->tm_wday;
	wday = (wday + 6) % 7;

	working_days_whole = 0;
	working_days_frac = 0;

	/*
	 * Treat the first day specially, in case it is a day of the
	 * weekend.
	 */
	if ((long)start + SECONDS_PER_WORKING_DAY <= (long)finish)
	{
		working_days_whole++;
		start += SECONDS_PER_DAY;
		wday = (wday + 1) % 7;
	}

	/*
	 * Loop over the intervening days, incrimenting the counter for
	 * any day that is not a day of the weekend.
	 */
	while ((long)start + SECONDS_PER_WORKING_DAY <= (long)finish)
	{
		if (wday < WORKING_DAYS_PER_WEEK)
			working_days_whole++;
		start += SECONDS_PER_DAY;
		wday = (wday + 1) % 7;
	}

	/*
	 * Always do the fraction, even if it is a day of the weekend.
	 */
	assert((long)finish - (long)start < SECONDS_PER_WORKING_DAY);
	if (start < finish)
	{
		working_days_frac =
			(finish - start) / (double)SECONDS_PER_WORKING_DAY;
	}

	/*
	 * done
	 */
	working_days_frac += working_days_whole;
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
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		rpt_value_free(t1);
		sub_var_set_charstar(scp, "Function", "working_days");
		sub_var_set_charstar(scp, "Number", "1");
		sub_var_set_charstar(scp, "Name", argv[0]->method->name);
		s =
			subst_intl
			(
				scp,
      i18n("$function: argument $number: time value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
		return result;
	}

	t2 = rpt_value_integerize(argv[1]);
	if (t2->method->type != rpt_value_type_integer)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		rpt_value_free(t1);
		rpt_value_free(t2);
		sub_var_set_charstar(scp, "Function", "working_days");
		sub_var_set_charstar(scp, "Number", "2");
		sub_var_set_charstar(scp, "Name", argv[1]->method->name);
		s =
			subst_intl
			(
				scp,
      i18n("$function: argument $number: time value required (was given $name)")
			);
		sub_context_delete(scp);
		result = rpt_value_error(ep->pos, s);
		str_free(s);
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
