//
// aegis - project change supervisor
// Copyright (C) 1994-1996, 1999, 2001-2008, 2012 Peter Miller
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

#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/void.h>
#include <libaegis/col.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/sub.h>


size_t rpt_func_print__ncolumns;
size_t rpt_func_print__ncolumns_max;
output::pointer *rpt_func_print__column;
col::pointer rpt_func_print__colp;


rpt_func_print::~rpt_func_print()
{
}


rpt_func_print::rpt_func_print()
{
}


rpt_func::pointer
rpt_func_print::create()
{
    return pointer(new rpt_func_print());
}


const char *
rpt_func_print::name()
    const
{
    return "print";
}


bool
rpt_func_print::optimizable()
    const
{
    return false;
}


bool
rpt_func_print::verify(const rpt_expr::pointer &ep)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!rpt_func_print__ncolumns)
    {
        trace(("no columns defined yet\n"));
        return false;
    }

    //
    // you may not print with more arguments
    // than columns defined
    //
    assert(ep);
    bool ok = (ep->get_nchildren() <= rpt_func_print__ncolumns);
    trace(("==> %d\n", ok));
    return ok;
}


rpt_value::pointer
rpt_func_print::run(const rpt_expr::pointer &ep, size_t argc,
    rpt_value::pointer *argv) const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (argc > (size_t)rpt_func_print__ncolumns)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "print");
        sc.var_set_long("Number1", (long)argc);
        sc.var_set_long("Number2", (long)rpt_func_print__ncolumns);
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: too many arguments ($number1 given, "
                    "only $number2 used)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // turn each argument into a string
    //
    rpt_value::pointer *argv2 = new rpt_value::pointer [argc];
    for (size_t j = 0; j < argc; ++j)
    {
        trace(("j = %d of %d\n", (int)j, (int)argc));
        rpt_value::pointer vp = rpt_value::stringize(argv[j]);
        trace(("mark\n"));
        rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(vp.get());
        if (rvsp)
        {
            trace(("ok\n"));
            argv2[j] = vp;
            continue;
        }

        delete [] argv2;

        //
        // ...and complain bitterly
        //
        trace(("erk\n"));
        sub_context_ty sc;
        sc.var_set_charstar("Function", "print");
        sc.var_set_long("Number", (long)j + 1);
        sc.var_set_charstar("Name", argv[j]->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: unable to print $name value")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // now that we know they are all stringizable, print the line out
    //
    for (size_t j = 0; j < argc; ++j)
    {
        trace(("j = %d of %d\n", (int)j, (int)argc));
        rpt_value::pointer vp = argv2[j];
        rpt_value_string *rvsp = dynamic_cast<rpt_value_string *>(vp.get());
        assert(rvsp);
        if (rpt_func_print__column[j])
        {
            rpt_func_print__column[j]->fputs(rvsp->query());
        }
    }
    delete [] argv2;

    //
    // Emit the line.
    //
    // The os_become bracketing is because we would write to the
    // file at this point, and some operating systems will barf if
    // we have the wrong uid.
    //
    trace(("emit\n"));
    os_become_orig();
    rpt_func_print__colp->eoln();
    os_become_undo();

    trace(("bye bye\n"));
    return rpt_value_void::create();
}


// vim: set ts=8 sw=4 et :
