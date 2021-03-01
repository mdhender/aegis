//
//      aegis - project change supervisor
//      Copyright (C) 1996, 1999, 2002, 2004-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/time.h>

#include <common/str.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/strftime.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


rpt_func_strftime::~rpt_func_strftime()
{
}


rpt_func_strftime::rpt_func_strftime()
{
}


rpt_func::pointer
rpt_func_strftime::create()
{
    return pointer(new rpt_func_strftime());
}


const char *
rpt_func_strftime::name()
    const
{
    return "strftime";
}


bool
rpt_func_strftime::optimizable()
    const
{
    return true;
}


bool
rpt_func_strftime::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 2);
}


rpt_value::pointer
rpt_func_strftime::run(const rpt_expr::pointer &ep, size_t argc,
    rpt_value::pointer *argv) const
{
    nstring t1("%C");
    if (argc >= 1)
    {
        rpt_value::pointer vp1 = rpt_value::stringize(argv[0]);
        rpt_value_string *vp1sp = dynamic_cast<rpt_value_string *>(vp1.get());
        if (!vp1sp)
        {
            sub_context_ty sc;
            sc.var_set_charstar("Function", "strftime");
            sc.var_set_long("Number", 1);
            sc.var_set_charstar("Name", argv[0]->name());
            nstring s
            (
                sc.subst_intl
                (
                    i18n("$function: argument $number: string value "
                        "required (was given $name)")
                )
            );
            return rpt_value_error::create(ep->get_pos(), s);
        }
        t1 = nstring(vp1sp->query());
    }

    time_t t2 = 0;
    if (argc >= 2)
    {
        rpt_value::pointer vp2 = rpt_value::integerize(argv[1]);
        rpt_value_integer *vp2ip = dynamic_cast<rpt_value_integer *>(vp2.get());
        if (!vp2ip)
        {
            sub_context_ty sc;
            sc.var_set_charstar("Function", "strftime");
            sc.var_set_long("Number", 2);
            sc.var_set_charstar("Name", argv[1]->name());
            nstring s
            (
                sc.subst_intl
                (
                    i18n("$function: argument $number: time value "
                        "required (was given $name)")
                )
            );
            return rpt_value_error::create(ep->get_pos(), s);
        }
        t2 = vp2ip->query();
    }

    char buffer[2000];
    int nbytes = strftime(buffer, sizeof(buffer), t1.c_str(), localtime(&t2));
    if (nbytes <= 0 || size_t(nbytes) > sizeof(buffer))
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "strftime");
        nstring s(sc.subst_intl(i18n("$function: result too long")));
        return rpt_value_error::create(ep->get_pos(), s);
    }

    return rpt_value_string::create(buffer);
}
