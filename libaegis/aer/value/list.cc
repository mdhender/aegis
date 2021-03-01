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

#include <common/error.h>
#include <common/mem.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/real.h>
#include <libaegis/sub.h>


rpt_value_list::~rpt_value_list()
{
    delete [] item;
    length = 0;
    max = 0;
    item = 0;
}


rpt_value_list::rpt_value_list() :
    length(0),
    max(0),
    item(0)
{
}


rpt_value::pointer
rpt_value_list::create()
{
    return pointer(new rpt_value_list());
}


rpt_value::pointer
rpt_value_list::lookup(const rpt_value::pointer &rhs, bool lvalue)
    const
{
    if (lvalue)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name1", name());
	sc.var_set_charstar("Name2", rhs->name());
	nstring s
        (
	    sc.subst_intl
	    (
		i18n("may not assign to a list member ($name1[$name2])")
	    )
        );
	return rpt_value_error::create(s);
    }
    rpt_value::pointer rhs2 = rpt_value::integerize(rhs);
    rpt_value_integer *rhs2ip = dynamic_cast<rpt_value_integer *>(rhs2.get());
    if (!rhs2ip)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Name1", name());
        sc.var_set_charstar("Name2", rhs->name());
        nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
        return rpt_value_error::create(s);
    }

    long idx = rhs2ip->query();
    if (idx < 0 || (size_t)idx >= length)
	return rpt_value_null::create();
    return item[idx];
}


rpt_value::pointer
rpt_value_list::keys()
    const
{
    rpt_value_list *p = new rpt_value_list();
    for (size_t j = 0; j < length; ++j)
    {
	p->append(rpt_value_integer::create(j));
    }
    return pointer(p);
}


rpt_value::pointer
rpt_value_list::count()
    const
{
    return rpt_value_integer::create(length);
}


const char *
rpt_value_list::name()
    const
{
    return "list";
}


void
rpt_value_list::append(const rpt_value::pointer &child)
{
    if (length >= max)
    {
	size_t new_max = max * 2 + 4;
	rpt_value::pointer *new_item = new rpt_value::pointer [new_max];
	for (size_t j = 0; j < length; ++j)
	    new_item[j] = item[j];
	delete [] item;
	item = new_item;
	max = new_max;
    }
    item[length++] = child;
}


rpt_value::pointer
rpt_value_list::nth(size_t n)
    const
{
    if (n >= length)
        return rpt_value_null::create();
    return item[n];
}
