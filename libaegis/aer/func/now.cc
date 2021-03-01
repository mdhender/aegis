//
// aegis - project change supervisor
// Copyright (C) 1994-1997, 1999, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/now.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/now.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/time.h>
#include <libaegis/sub.h>


rpt_func_now::~rpt_func_now()
{
}


rpt_func_now::rpt_func_now()
{
}


rpt_func::pointer
rpt_func_now::create()
{
    return pointer(new rpt_func_now());
}


const char *
rpt_func_now::name()
    const
{
    return "now";
}


bool
rpt_func_now::optimizable()
    const
{
    return false;
}


bool
rpt_func_now::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_now::run(const rpt_expr::pointer &, size_t, rpt_value::pointer *)
    const
{
    return rpt_value_time::create(now());
}


#define WORKING_DAYS_PER_WEEK 5
#define HOURS_PER_WORKING_DAY 7.5
#define SECONDS_PER_WORKING_DAY (long)(HOURS_PER_WORKING_DAY * 60L * 60L)
#define SECONDS_PER_DAY (24L * 60L * 60L)


double
working_days(time_t start, time_t finish)
{
    //
    // Flip it end-for-end if they gave it the wrong way round.
    //
    trace(("working_days(start = %ld, finish = %ld)\n{\n",
           (long)start, (long)finish));
    trace(("start = %s", ctime(&start)));
    trace(("finish = %s", ctime(&finish)));
    if (start > finish)
    {
        time_t tmp = start;
        finish = start;
        start = tmp;
    }

    //
    // Get the current week say.
    // Adjust it so that MON=0 thru SUN=6
    //
    int wday = localtime(&start)->tm_wday;
    wday = (wday + 6) % 7;

    long working_days_whole = 0;
    double working_days_frac = 0;

    //
    // Treat the first day specially, in case it is a day of the
    // weekend.
    //
    if ((long)start + SECONDS_PER_WORKING_DAY <= (long)finish)
    {
        working_days_whole++;
        start += SECONDS_PER_DAY;
        wday = (wday + 1) % 7;
    }

    //
    // Loop over the intervening days, incrimenting the counter for
    // any day that is not a day of the weekend.
    //
    while ((long)start + SECONDS_PER_WORKING_DAY <= (long)finish)
    {
        if (wday < WORKING_DAYS_PER_WEEK)
            working_days_whole++;
        start += SECONDS_PER_DAY;
        wday = (wday + 1) % 7;
    }

    //
    // Always do the fraction, even if it is a day of the weekend.
    //
    assert((long)finish - (long)start < SECONDS_PER_WORKING_DAY);
    if (start < finish)
    {
        working_days_frac = (finish - start) / (double)SECONDS_PER_WORKING_DAY;
    }

    //
    // done
    //
    working_days_frac += working_days_whole;
    trace(("return %.10g;\n", working_days_frac));
    trace(("}\n"));
    return working_days_frac;
}


rpt_func_working_days::~rpt_func_working_days()
{
}


rpt_func_working_days::rpt_func_working_days()
{
}


rpt_func::pointer
rpt_func_working_days::create()
{
    return pointer(new rpt_func_working_days());
}


const char *
rpt_func_working_days::name()
    const
{
    return "working_days";
}


bool
rpt_func_working_days::optimizable()
    const
{
    return true;
}


bool
rpt_func_working_days::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 2);
}


rpt_value::pointer
rpt_func_working_days::run(const rpt_expr::pointer &ep, size_t,
    rpt_value::pointer *argv) const
{
    rpt_value::pointer t1 = rpt_value::integerize(argv[0]);
    rpt_value_integer *t1ip = dynamic_cast<rpt_value_integer *>(t1.get());
    if (!t1ip)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "working_days");
        sc.var_set_long("Number", 1);
        sc.var_set_charstar("Name", argv[0]->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: time value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    rpt_value::pointer t2 = rpt_value::integerize(argv[1]);
    rpt_value_integer *t2ip = dynamic_cast<rpt_value_integer *>(t2.get());
    if (!t2ip)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "working_days");
        sc.var_set_long("Number", 2);
        sc.var_set_charstar("Name", argv[1]->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: time value required "
                    "(was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    return rpt_value_real::create(working_days(t1ip->query(), t2ip->query()));
}


// vim: set ts=8 sw=4 et :
