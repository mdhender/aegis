//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2003-2008 Peter Miller.
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

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/columns.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/col.h>
#include <libaegis/output.h>


rpt_func_columns::~rpt_func_columns()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_func_columns::rpt_func_columns()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_func::pointer
rpt_func_columns::create()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_func_columns());
}


const char *
rpt_func_columns::name()
    const
{
    return "columns";
}


bool
rpt_func_columns::optimizable()
    const
{
    return false;
}


bool
rpt_func_columns::verify(const rpt_expr::pointer &ep)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return (ep->get_nchildren() > 0);
}


static long
extract_integer(const rpt_value::pointer &vp, int minimum_result)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!vp)
        return -1;
    if (dynamic_cast<const rpt_value_null *>(vp.get()))
        return -1;
    rpt_value::pointer vp3 = rpt_value::integerize(vp);

    rpt_value_integer *ivp = dynamic_cast<rpt_value_integer *>(vp3.get());
    if (ivp)
    {
	long result = ivp->query();
        if (result < minimum_result)
        {
            trace(("%ld < %d ==> -1\n", result, minimum_result));
            return -1;
        }
        trace(("==> %ld\n", result));
        return result;
    }

    //
    // If all else fails, cry.
    //
    trace(("not an int => -1\n"));
    return -1;
}


static nstring
extract_string(const rpt_value::pointer &vp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!vp)
	return "";
    rpt_value::pointer vp3 = rpt_value::stringize(vp);

    rpt_value_string *svp = dynamic_cast<rpt_value_string *>(vp3.get());
    if (svp)
        return svp->query();

    return "";
}


rpt_value::pointer
rpt_func_columns::run(const rpt_expr::pointer &, size_t argc,
    rpt_value::pointer *argv) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    //
    // Nuke the old columns.
    //
    while (rpt_func_print__ncolumns > 0)
    {
	trace(("mark\n"));
	--rpt_func_print__ncolumns;
	size_t j = rpt_func_print__ncolumns;
	rpt_func_print__column[j].reset();
    }

    //
    // Allocate more space if we need it.
    //
    if (argc >= rpt_func_print__ncolumns_max)
    {
	trace(("argc = %d\n", (int)argc));
	size_t new_rpt_func_print__ncolumns_max =
	    rpt_func_print__ncolumns_max * 2 + 8;
	while (argc >= new_rpt_func_print__ncolumns_max)
	{
	    new_rpt_func_print__ncolumns_max =
		new_rpt_func_print__ncolumns_max * 2 + 8;
	}
	trace(("new max = %d\n", (int)new_rpt_func_print__ncolumns_max));
	output::pointer *new_rpt_func_print__column =
	    new output::pointer [new_rpt_func_print__ncolumns_max];
	size_t k = 0;
	for (; k < rpt_func_print__ncolumns_max; ++k)
	    new_rpt_func_print__column[k] = rpt_func_print__column[k];
	delete [] rpt_func_print__column;
	rpt_func_print__column = new_rpt_func_print__column;
	rpt_func_print__ncolumns_max = new_rpt_func_print__ncolumns_max;
    }

    //
    // Create each of the columns.
    //
    trace(("create each of the columns\n"));
    assert(rpt_func_print__colp);
    int right = 0;
    int padding_prev = 0;
    for (size_t j = 0; j < argc; ++j)
    {
	trace(("j = %d of %d\n", (int)j, (int)argc));
        rpt_value::pointer vp = argv[j];
        nstring nname;
        int left = -1;
        int padding = -1;
        int rightm = -1;
        int width = -1;

        rpt_value_struct *rvs = dynamic_cast<rpt_value_struct *>(vp.get());
	if (rvs)
	{
            trace(("{ struct }\n"));
	    rpt_value::pointer vp2 = rvs->lookup("left");
	    left = extract_integer(vp2, 0);
            trace(("left = %d\n", left));

	    vp2 = rvs->lookup("name");
	    nname = extract_string(vp2);

	    vp2 = rvs->lookup("padding");
	    padding = extract_integer(vp2, 0);
            trace(("padding = %d\n", padding));

	    vp2 = rvs->lookup("right");
	    rightm = extract_integer(vp2, 0);
            trace(("rightm = %d\n", rightm));

	    vp2 = rvs->lookup("width");
	    width = extract_integer(vp2, 1);
            trace(("width = %d\n", width));
	}
	else
	{
	    vp = rpt_value::stringize(vp);
	    nname = extract_string(vp);
	}

	//
	// padding defaults to 1
	//	except on the left margin
	//
	if (padding < 0)
	    padding = (right != 0);
	trace(("padding = %d\n", padding));

	//
	// left defaults to the right of the previous column
	//
	// padding applies on the left and on the right
	//
	if (left < 0)
	{
	    if (rightm > 0 && width > 0 && rightm >= right + width)
            {
                trace(("left = rightm - width\n"));
	       	left = rightm - width;
            }
	    else if (right)
	    {
	       	if (padding > padding_prev)
                {
		    trace(("left = right + padding;\n"));
		    left = right + padding;
                }
	       	else
                {
		    trace(("left = right + padding_prev;\n"));
		    left = right + padding_prev;
                }
	    }
	    else
            {
                trace(("left = edge;\n"));
	       	left = 0;
            }
	}
	trace(("left = %d\n", left));

	//
	// the width defaults to 7
	// (default padding + default width == 8, so tabs work)
	//
	if (width < 0)
	{
	    if (rightm > left)
	       	width = rightm - left;
	    else
	       	width = 7;
	}
	trace(("width = %d\n", width));
	if (rightm == 0)
	    right = rightm;
	else
	    right = left + width;
	trace(("right = %d\n", right));

	//
	// create the column and set its heading
	//
	rpt_func_print__column[j] =
	    rpt_func_print__colp->create(left, right, nname.c_str());

	//
	// setup for next column
	//
	padding_prev = padding;
	trace(("mark\n"));
    }
    rpt_func_print__ncolumns = argc;

    //
    // done
    //
    trace(("done\n"));
    return rpt_value_null::create();
}
