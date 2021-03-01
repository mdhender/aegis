//
//	aegis - project change supervisor
//	Copyright (C) 1997, 2002-2008 Peter Miller
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

#include <common/str.h>
#include <libaegis/aer/value/string.h>

#include <aefind/tree/constant.h>


tree_constant::~tree_constant()
{
}


tree_constant::tree_constant(rpt_value::pointer a_value) :
    value(a_value)
{
}


tree::pointer
tree_constant::create(rpt_value::pointer a_value)
{
    return pointer(new tree_constant(a_value));
}


void
tree_constant::print()
    const
{
    rpt_value::pointer vp = rpt_value::stringize(value);
    rpt_value_string *ss = dynamic_cast<rpt_value_string *>(vp.get());
    if (ss)
        printf("'%s'", ss->query().c_str());
    else
        printf("''");
}


rpt_value::pointer
tree_constant::evaluate(string_ty *, string_ty *, string_ty *, struct stat *)
    const
{
    return value;
}


bool
tree_constant::useful()
    const
{
    return false;
}


bool
tree_constant::constant()
    const
{
    return true;
}


const char *
tree_constant::name()
    const
{
    return "constant";
}


tree::pointer
tree_constant::optimize()
    const
{
    return create(value);
}
