//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2003-2008 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/mem.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/sort.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/sub.h>


#define PAIR(a, b) ((a) * rpt_value_type_MAX + (b))


rpt_func_sort::~rpt_func_sort()
{
}


rpt_func_sort::rpt_func_sort()
{
}


rpt_func::pointer
rpt_func_sort::create()
{
    return pointer(new rpt_func_sort());
}


const char *
rpt_func_sort::name()
    const
{
    return "sort";
}


bool
rpt_func_sort::optimizable()
    const
{
    return true;
}


bool
rpt_func_sort::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 1);
}


static int
cmp(const void *va, const void *vb)
{
    rpt_value::pointer a = *(rpt_value::pointer *)va;
    rpt_value::pointer b = *(rpt_value::pointer *)vb;

    rpt_value_integer *aip = dynamic_cast<rpt_value_integer *>(a.get());
    if (aip)
    {
        long na = aip->query();

        rpt_value_integer *bip = dynamic_cast<rpt_value_integer *>(b.get());
        if (bip)
        {
            long nb = bip->query();
            if (na < nb)
                return -1;
            if (na > nb)
                return 1;
            return 0;
        }

        rpt_value_real *brp = dynamic_cast<rpt_value_real *>(b.get());
        if (brp)
        {
            double nb = brp->query();
            if (na < nb)
                return -1;
            if (na > nb)
                return 1;
            return 0;
        }
    }

    rpt_value_real *arp = dynamic_cast<rpt_value_real *>(a.get());
    if (arp)
    {
        double na = arp->query();

        rpt_value_integer *bip = dynamic_cast<rpt_value_integer *>(b.get());
        if (bip)
        {
            long nb = bip->query();
            if (na < nb)
                return -1;
            if (na > nb)
                return 1;
            return 0;
        }

        rpt_value_real *brp = dynamic_cast<rpt_value_real *>(b.get());
        if (brp)
        {
            double nb = brp->query();
            if (na < nb)
                return -1;
            if (na > nb)
                return 1;
            return 0;
        }
    }

    //
    // compare strings, if possible
    //
    rpt_value_string *asp = dynamic_cast<rpt_value_string *>(a.get());
    rpt_value_string *bsp = dynamic_cast<rpt_value_string *>(b.get());
    if (asp && bsp)
        return strcmp(asp->query().c_str(), bsp->query().c_str());

    //
    // absolutely no idea
    //
    return 0;
}


rpt_value::pointer
rpt_func_sort::run(const rpt_expr::pointer &ep, size_t argc,
    rpt_value::pointer *argv) const
{
    if (argc != 1)
    {
        sub_context_ty sc;
        sc.var_set_long("Number", (long)argc);
        nstring s
        (
            sc.subst_intl
            (
                i18n("sort requires exactly 1 argument (was given $number)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }
    rpt_value::pointer vp = argv[0];
    rpt_value_list *rvlp = dynamic_cast<rpt_value_list *>(vp.get());
    if (!rvlp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("sort requires a list argument (was given $name)")
            )
        );
        return rpt_value_error::create(ep->get_pos(), s);
    }

    //
    // sort the list items
    //
    size_t length = rvlp->size();
    rpt_value::pointer *item = new rpt_value::pointer [length];
    for (size_t j = 0; j < length; ++j)
        item[j] = rpt_value::arithmetic(rvlp->nth(j));
    qsort(item, length, sizeof(item[0]), cmp);

    //
    // build the result
    //
    rpt_value_list *rlp = new rpt_value_list();
    rpt_value::pointer result(rlp);
    for (size_t k = 0; k < length; ++k)
        rlp->append(item[k]);
    delete [] item;
    return result;
}
