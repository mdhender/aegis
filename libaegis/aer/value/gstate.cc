//
// aegis - project change supervisor
// Copyright (C) 1994-1999, 2000, 2003-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see
// <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h> // for strerror()

#include <common/error.h>
#include <common/mem.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/gstate.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/pstate.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/change.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


rpt_value_gstate::~rpt_value_gstate()
{
}


rpt_value_gstate::rpt_value_gstate()
{
}


rpt_value::pointer
rpt_value_gstate::create()
{
    static rpt_value::pointer vp;
    if (!vp)
        vp = pointer(new rpt_value_gstate());
    return vp;
}


rpt_value::pointer
grab_one(const nstring &project_name)
{
    //
    // get details of the project
    // to put in the structure
    //
    project *pp = project_alloc(project_name.get_ref());
    pp->bind_existing();
    rpt_value_struct *rsp = new rpt_value_struct();
    rpt_value::pointer result(rsp);

    nstring pn(project_name_get(pp));
    rpt_value::pointer value = rpt_value_string::create(pn);
    rsp->assign("name", value);

    int err = project_is_readable(pp);
    if (err)
    {
        rsp->assign("error", rpt_value_string::create(strerror(err)));
    }
    else
    {
        //
        // The development directory of the project change is
        // the one which contains the trunk or branch baseline.
        //
        change::pointer cp = pp->change_get();
        if (cp->is_being_developed())
        {
            nstring dd(change_development_directory_get(cp, 0));
            rsp->assign("directory", rpt_value_string::create(dd));
        }

        value = rpt_value_pstate::create(project_name_get(pp));
        rsp->assign("state", value);
    }

    project_free(pp);

    //
    // all done
    //
    return result;
}


rpt_value::pointer
rpt_value_gstate::lookup(const rpt_value::pointer &rhs, bool)
    const
{
    trace(("rpt_value_gstate::lookup()\n"));
    rpt_value::pointer rhs2 = rpt_value::stringize(rhs);
    trace(("rhs2 is a \"%s\"\n", rhs->name()));
    rpt_value_string *rhs2sp = dynamic_cast<rpt_value_string *>(rhs2.get());
    if (!rhs2sp)
    {
        trace(("keys is not a string\n"));
        sub_context_ty sc;
        sc.var_set_charstar("Name1", "project");
        sc.var_set_charstar("Name2", rhs->name());
        nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
        return rpt_value_error::create(s);
    }

    return grab_one(rhs2sp->query());
}


void
rpt_value_gstate::grab()
    const
{
    if (vkeys)
        return;

    //
    // create a structure/array to hold the thing
    //
    trace(("rpt_value_gstate::grab(this = %p)\n{\n", this));

    //
    // ask gonzo for the list of project names
    //
    string_list_ty names;
    project_list_get(&names);
    rpt_value_list *p = new rpt_value_list();
    vkeys = rpt_value::pointer(p);
    for (size_t j = 0; j < names.size(); ++j)
    {
        nstring s(names[j]);
        p->append(rpt_value_string::create(s));
    }
    trace(("}\n"));
}


rpt_value::pointer
rpt_value_gstate::keys()
    const
{
    trace(("rpt_value_gstate::keys(this = %p)\n{\n", this));
    grab();
    return vkeys;
}


rpt_value::pointer
rpt_value_gstate::count()
    const
{
    trace(("rpt_value_gstate::count(this = %p)\n", this));
    grab();
    return vkeys->count();
}


const char *
rpt_value_gstate::type_of()
    const
{
    return "struct";
}


const char *
rpt_value_gstate::name()
    const
{
    return "gstate";
}


bool
rpt_value_gstate::is_a_struct()
    const
{
    return true;
}


// vim: set ts=8 sw=4 et :
