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

#include <libaegis/introspector/nstring.h>


introspector_nstring::~introspector_nstring()
{
}


introspector_nstring::introspector_nstring(nstring &a_where) :
    where(a_where)
{
}


introspector::pointer
introspector_nstring::create(nstring &a_where)
{
    return pointer(new introspector_nstring(a_where));
}


nstring
introspector_nstring::get_name()
    const
{
    return "string";
}


void
introspector_nstring::string(const nstring &text)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    where = text;
}
