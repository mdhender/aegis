//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/nstring.h>
#include <libaegis/introspector/error.h>


introspector_error::~introspector_error()
{
}


introspector_error::introspector_error()
{
}


introspector_error::pointer
introspector_error::create()
{
    return pointer(new introspector_error());
}


void
introspector_error::integer(long)
{
}


void
introspector_error::real(double)
{
}


void
introspector_error::string(const nstring &)
{
}


introspector::pointer
introspector_error::list()
{
    return create();
}


introspector::pointer
introspector_error::field(const nstring &)
{
    return create();
}


nstring
introspector_error::get_name()
    const
{
    //
    // No code path should result in this method being called, because
    // this class implements all methods and silently ignored them.
    // This is to suppress secondary error messages.
    //
    assert(0);
    return "error";
}


// vim: set ts=8 sw=4 et :
