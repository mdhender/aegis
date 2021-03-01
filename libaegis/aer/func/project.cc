//
//      aegis - project change supervisor
//      Copyright (C) 1994, 2003-2008, 2011, 2012 Peter Miller.
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

#include <common/ac/assert.h>

#include <common/str.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/project.h>
#include <libaegis/aer/value/boolean.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/change/identifier.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


static change_identifier *cidp;


void
report_parse_project_set(change_identifier &cid)
{
    assert(!cidp);
    cidp = &cid;
}


rpt_func_project_name::~rpt_func_project_name()
{
}


rpt_func_project_name::rpt_func_project_name()
{
}


rpt_func::pointer
rpt_func_project_name::create()
{
    return pointer(new rpt_func_project_name());
}


const char *
rpt_func_project_name::name()
    const
{
    return "project_name";
}


bool
rpt_func_project_name::optimizable()
    const
{
    return false;
}


bool
rpt_func_project_name::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_project_name::run(const rpt_expr::pointer &, size_t,
    rpt_value::pointer *) const
{
    assert(cidp);
    project *pp = cidp->get_pp();
    if (cidp->get_baseline())
        pp = pp->parent_get();
    return rpt_value_string::create(nstring(pp->name_get()));
}


rpt_func_project_name_set::~rpt_func_project_name_set()
{
}


rpt_func_project_name_set::rpt_func_project_name_set()
{
}


rpt_func::pointer
rpt_func_project_name_set::create()
{
    return pointer(new rpt_func_project_name_set());
}


const char *
rpt_func_project_name_set::name()
    const
{
    return "project_name_set";
}


bool
rpt_func_project_name_set::optimizable()
    const
{
    return false;
}


bool
rpt_func_project_name_set::verify(const rpt_expr::pointer &ep)
    const
{
    return (ep->get_nchildren() == 0);
}


rpt_value::pointer
rpt_func_project_name_set::run(const rpt_expr::pointer &, size_t,
    rpt_value::pointer *) const
{
    assert(cidp);
    return rpt_value_boolean::create(cidp->project_set());
}


// vim: set ts=8 sw=4 et :
