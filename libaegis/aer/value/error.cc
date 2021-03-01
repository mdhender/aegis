//
//      aegis - project change supervisor
//      Copyright (C) 1994-1996, 1999, 2003-2008 Peter Miller
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

#include <common/error.h>
#include <libaegis/aer/pos.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/sub.h>


rpt_value_error::~rpt_value_error()
{
}


rpt_value_error::rpt_value_error(const rpt_position::pointer &a_where,
        const nstring &a_what) :
    where(a_where),
    what(a_what)
{
}


rpt_value::pointer
rpt_value_error::create(const rpt_position::pointer &e_where, string_ty *e_what)
{
    return pointer(new rpt_value_error(e_where, nstring(e_what)));
}


rpt_value::pointer
rpt_value_error::create(const rpt_position::pointer &e_where,
    const nstring &e_what)
{
    return pointer(new rpt_value_error(e_where, e_what));
}


rpt_value::pointer
rpt_value_error::create(string_ty *e_what)
{
    return create(rpt_position::pointer(), nstring(e_what));
}


rpt_value::pointer
rpt_value_error::create(const nstring &e_what)
{
    return create(rpt_position::pointer(), e_what);
}


const char *
rpt_value_error::name()
    const
{
    return "error";
}


bool
rpt_value_error::is_an_error()
    const
{
    return true;
}


nstring
rpt_value_error::query()
    const
{
    return what;
}


void
rpt_value_error::print()
    const
{
    sub_context_ty sc;
    sc.var_set_string("MeSsaGe", what);
    if (where)
        where->print_error(sc, i18n("$message"));
    else
        sc.error_intl(i18n("$message"));
}


void
rpt_value_error::setpos(const rpt_position::pointer &a_where)
{
    if (!where)
        where = a_where;
}
