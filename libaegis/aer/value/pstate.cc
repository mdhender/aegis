//
//	aegis - project change supervisor
//	Copyright (C) 1994-1997, 1999, 2002-2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/pos.h>
#include <libaegis/aer/value/cstate.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/fstate.h>
#include <libaegis/aer/value/pstate.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/change.h>
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


rpt_value_pstate::~rpt_value_pstate()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value_pstate::rpt_value_pstate(const nstring &a_pname) :
    pname(a_pname)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


rpt_value::pointer
rpt_value_pstate::create(const nstring &a_pname)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_value_pstate(a_pname));
}


rpt_value::pointer
rpt_value_pstate::create(string_ty *a_pname)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return create(nstring(a_pname));
}


void
rpt_value_pstate::grab()
    const
{
    //
    // construct the project, assuming it exists
    //
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(!value);
    assert(!pname.empty());
    project_ty *pp = project_alloc(pname.get_ref());
    pp->bind_existing();

    //
    // make sure the project state is readable
    //
    int err = project_is_readable(pp);
    if (err)
    {
	sub_context_ty sc;
	sc.errno_setx(err);
	sc.var_set_string("File_Name", pp->pstate_path_get());
	nstring s(sc.subst_intl("stat $filename: $errno"));
	value = rpt_value_error::create(s);
	return;
    }

    //
    // create the result value
    //
    change::pointer cp = pp->change_get();
    cstate_ty *cstate_data = cp->cstate_get();
    value = cstate_type.convert(&cstate_data);
    assert(value);
    rpt_value_struct *vp = dynamic_cast<rpt_value_struct *>(value.get());
    assert(vp);

    //
    // The src field is now kept in the fstate file.
    // It will not be read in until referenced.
    //
    assert(!cstate_data->src);
    vp->assign("src", rpt_value_fstate::create(cp));

    //
    // add some extra stuff
    //
    vp->assign("name", rpt_value_string::create(nstring(project_name_get(pp))));

    if (cstate_data->state == cstate_state_being_developed)
    {
        nstring dd(change_development_directory_get(cp, 0));
	vp->assign("directory", rpt_value_string::create(dd));
    }

    if (!pp->is_a_trunk())
    {
        nstring pn(project_name_get(pp->parent_get()));
	vp->assign("parent_name", rpt_value_string::create(pn));
    }

    //
    // mangle the "change" list
    // into a deferred array of cstate values
    //
    if (cstate_data->branch && cstate_data->branch->change)
    {
	rpt_value::pointer vp1 = rpt_value_string::create("branch");
	rpt_value::pointer vp2 = value->lookup(vp1, false);
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
	vp2sp->assign("change", vp3);
    }

    //
    // clean up and go home
    //
    project_free(pp);
}


rpt_value::pointer
rpt_value_pstate::lookup(const rpt_value::pointer &rhs, bool lval)
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!value)
	grab();
    assert(value);
    if (value->is_an_error())
        return value;

    return value->lookup(rhs, lval);
}


rpt_value::pointer
rpt_value_pstate::keys()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!value)
	grab();
    assert(value);
    if (value->is_an_error())
        return value;

    return value->keys();
}


rpt_value::pointer
rpt_value_pstate::count()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!value)
	grab();
    assert(value);
    if (value->is_an_error())
        return value;

    return value->count();
}


const char *
rpt_value_pstate::type_of()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!value)
	grab();
    assert(value);
    return value->type_of();
}


rpt_value::pointer
rpt_value_pstate::undefer_or_null()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (!value)
	grab();
    assert(value);
    return value;
}


const char *
rpt_value_pstate::name()
    const
{
    return "pstate";
}


bool
rpt_value_pstate::is_a_struct()
    const
{
    if (!value)
        return true;
    return value->is_a_struct();
}
