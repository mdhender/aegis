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

#include <common/nstring.h>
#include <common/trace.h>

#include <libaegis/introspector/string.h>


introspector_string::~introspector_string()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


introspector_string::introspector_string(string_ty *&a_where) :
    where(a_where)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


introspector_string::pointer
introspector_string::create(string_ty *&a_where)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new introspector_string(a_where));
}


nstring
introspector_string::get_name()
    const
{
    return "string";
}


void
introspector_string::string(const nstring &text)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (where)
        str_free(where);
    where = text.get_ref_copy();
}
