//
//	aegis - project change supervisor
//	Copyright (C) 1994-1997, 1999, 2001-2008 Peter Miller
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

#include <common/ac/stdio.h>

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/group.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/getgr_cache.h>
#include <libaegis/sub.h>


rpt_value_group::~rpt_value_group()
{
}


rpt_value_group::rpt_value_group()
{
}


rpt_value::pointer
rpt_value_group::create()
{
    static rpt_value::pointer vp;
    if (!vp)
        vp = pointer(new rpt_value_group());
    return vp;
}


static rpt_value::pointer
gr_to_struct(struct group *gr)
{
    trace(("gr_to_struct()\n{\n"));
    rpt_value_struct *p = new rpt_value_struct();
    rpt_value::pointer result(p);

    trace(("gr_name\n"));
    nstring name = "gr_name";
    rpt_value::pointer value = rpt_value_string::create(gr->gr_name);
    p->assign(name, value);

    trace(("gr_gid\n"));
    name = "gr_gid";
    value = rpt_value_integer::create(gr->gr_gid);
    p->assign(name, value);

    trace(("gr_mem\n"));
    name = "gr_mem";
    rpt_value_list *lp = new rpt_value_list();
    value = rpt_value::pointer(lp);
    p->assign(name, value);
    for (size_t j = 0; gr->gr_mem[j]; ++j)
    {
	rpt_value::pointer vp = rpt_value_string::create(gr->gr_mem[j]);
	lp->append(vp);
    }

    trace(("return %8.8lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_value::pointer
rpt_value_group::lookup(const rpt_value::pointer &rhs, bool)
    const
{
    trace(("value_group::lookup()\n"));
    rpt_value::pointer rhs2 = rpt_value::arithmetic(rhs);
    rpt_value_integer *rhs2ip = dynamic_cast<rpt_value_integer *>(rhs2.get());
    if (rhs2ip)
    {
	int gid = rhs2ip->query();
	struct group *gr = getgrgid_cached(gid);
	if (gr)
	    return gr_to_struct(gr);

        sub_context_ty sc;
        sc.var_set_long("Number", gid);
        nstring s(sc.subst_intl(i18n("gid $number unknown")));
        return rpt_value_error::create(s);
    }

    rhs2 = rpt_value::stringize(rhs);
    rpt_value_string *rhs2sp = dynamic_cast<rpt_value_string *>(rhs2.get());
    if (rhs2sp)
    {
        nstring gname = rhs2sp->query();
        struct group *gr = getgrnam_cached(gname);
        if (gr)
            return gr_to_struct(gr);

        sub_context_ty sc;
        sc.var_set_string("Name", gname);
        nstring s(sc.subst_intl(i18n("group \"$name\" unknown")));
        return rpt_value_error::create(s);
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", name());
    sc.var_set_charstar("Name2", rhs->name());
    nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
rpt_value_group::keys()
    const
{
    rpt_value_list *p = new rpt_value_list;
    rpt_value::pointer result(p);
    setgrent();
    for (;;)
    {
	struct group *gr = getgrent();
	if (!gr)
	    break;
        p->append(rpt_value_string::create(gr->gr_name));
    }
    return result;
}


rpt_value::pointer
rpt_value_group::count()
    const
{
    setgrent();
    long n = 0;
    for (;;)
    {
	struct group *gr = getgrent();
	if (!gr)
	    break;
	++n;
    }
    return rpt_value_integer::create(n);
}


const char *
rpt_value_group::type_of()
    const
{
    return "struct";
}


const char *
rpt_value_group::name()
    const
{
    return "group";
}


bool
rpt_value_group::is_a_struct()
    const
{
    return true;
}
