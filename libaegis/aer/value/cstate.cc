//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2001-2008 Peter Miller
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
#include <common/trace.h>
#include <libaegis/aer/value/cstate.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/fstate.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/null.h>
#include <libaegis/aer/value/pconf.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/change.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


rpt_value_cstate::~rpt_value_cstate()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (pp)
        project_free(pp);
    delete [] list;
}


rpt_value_cstate::rpt_value_cstate(project_ty *a_pp, size_t a_length,
        const long *a_list) :
    pp(project_copy(a_pp)),
    length(a_length),
    list(0)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    list = new long [length];
    for (size_t j = 0; j < length; ++j)
	list[j] = magic_zero_decode(a_list[j]);
}


rpt_value::pointer
rpt_value_cstate::create(project_ty *pptr, size_t o_length, const long *vlist)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_value_cstate(pptr, o_length, vlist));
}


rpt_value::pointer
rpt_value_cstate::lookup(const rpt_value::pointer &rhs, bool)
    const
{
    //
    // extract the change number
    //
    trace(("rpt_value_cstate::lookup(this = %08lX)\n{\n", (long)this));
    rpt_value::pointer rhs2 = rpt_value::integerize(rhs);
    rpt_value_integer *rhs2ip = dynamic_cast<rpt_value_integer *>(rhs2.get());
    if (!rhs2ip)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", rhs2->name());
	nstring s
        (
            sc.subst_intl(i18n("integer index required (was given $name)"))
        );
	rpt_value::pointer result = rpt_value_error::create(s);
	trace(("}\n"));
	return result;
    }
    long change_number = rhs2ip->query();
    trace(("change_number = %ld;\n", change_number));

    //
    // see if the change exists
    //
    size_t j = 0;
    for (j = 0; j < length; ++j)
	if (list[j] == change_number)
    	    break;
    if (j >= length)
    {
        trace(("change number not found\n"));
	trace(("}\n"));
	return rpt_value_null::create();
    }

    //
    // find the change
    //
    change::pointer cp = change_alloc(pp, magic_zero_encode(change_number));
    change_bind_existing(cp);
    trace(("cp = %08lX;\n", (long)cp));

    //
    // create the result value
    //
    cstate_ty *cstate_data = cp->cstate_get();
    rpt_value::pointer result = cstate_type.convert(&cstate_data);
    assert(result);
    rpt_value_struct *result_struct_p =
        dynamic_cast<rpt_value_struct *>(result.get());
    assert(result_struct_p);

    //
    // The src field is now kept in the fstate file.
    // It will not be read in until referenced.
    //
    assert(!cstate_data->src);
    nstring sname("src");
    rpt_value::pointer vp1 = rpt_value_fstate::create(cp);
    result_struct_p->assign(sname, vp1);

    //
    // add some extra stuff
    //
    sname = "project_name";
    vp1 = rpt_value_string::create(nstring(project_name_get(pp)));
    result_struct_p->assign(sname, vp1);

    sname = "change_number";
    vp1 = rpt_value_integer::create(change_number);
    result_struct_p->assign(sname, vp1);

    //
    // Add a special "config" field, which is a deferred reference
    // to the project config data.
    //
    sname = "config";
    vp1 = rpt_value_pconf::create(cp);
    result_struct_p->assign(sname, vp1);

    //
    // mangle the "change" list
    // into a deferred array of cstate values
    //
    if (cstate_data->branch && cstate_data->branch->change)
    {
	sname = "branch";
	vp1 = rpt_value_string::create(sname);
	assert(vp1);
	rpt_value::pointer vp2 = result->lookup(vp1, false);
	assert(vp2);
        rpt_value_struct *vp2sp = dynamic_cast<rpt_value_struct *>(vp2.get());
        assert(vp2sp);

	rpt_value::pointer vp3 =
	    rpt_value_cstate::create
	    (
	       	pp,
	       	cstate_data->branch->change->length,
	       	cstate_data->branch->change->list
	    );
	sname = "change";
	vp2sp->assign(sname, vp3);
    }

    //
    // clean up and go home
    //
    change_free(cp);
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_value::pointer
rpt_value_cstate::keys()
    const
{
    trace(("rpt_value_cstate::keys(this = %08lX)\n{\n", (long)this));
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer result(p);
    for (size_t j = 0; j < length; ++j)
    {
	rpt_value::pointer elem =
            rpt_value_integer::create(magic_zero_decode(list[j]));
	p->append(elem);
    }
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_value::pointer
rpt_value_cstate::count()
    const
{
    trace(("rpt_value_cstate::count(this = %08lX)\n{\n", (long)this));
    rpt_value::pointer result = rpt_value_integer::create(length);
    trace(("return %08lX;\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


const char *
rpt_value_cstate::type_of()
    const
{
    trace(("rpt_value_cstate::type_of()\n{\n"));
    const char *result = "struct";
    trace(("return \"%s\";\n", result));
    trace(("}\n"));
    return result;
}


bool
rpt_value_cstate::is_a_struct()
    const
{
    return true;
}


const char *
rpt_value_cstate::name()
    const
{
    return "cstate";
}
