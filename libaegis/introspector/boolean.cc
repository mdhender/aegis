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

#include <libaegis/introspector/boolean.h>


introspector_boolean::~introspector_boolean()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


introspector_boolean::introspector_boolean()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


nstring
introspector_boolean::get_name()
    const
{
    return "boolean";
}
