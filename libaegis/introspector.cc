//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/trace.h>

#include <libaegis/introspector.h>
#include <libaegis/introspector/error.h>
#include <libaegis/meta_lex.h>
#include <libaegis/sub.h>


introspector::~introspector()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


introspector::introspector()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


void
introspector::error(sub_context_ty *scp, const char *text)
{
    lex_error(scp, text);
}


void
introspector::value_of_type_required()
{
    sub_context_ty sc;
    sc.var_set_string("Name", get_name());
    error(&sc, i18n("value of type $name required"));
}


void
introspector::integer(long)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value_of_type_required();
}


void
introspector::real(double)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value_of_type_required();
}


void
introspector::string(const nstring &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value_of_type_required();
}


void
introspector::enumeration(const nstring &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value_of_type_required();
}


introspector::pointer
introspector::list()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value_of_type_required();
    return introspector_error::create();
}


introspector::pointer
introspector::field(const nstring &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    value_of_type_required();
    return introspector_error::create();
}
