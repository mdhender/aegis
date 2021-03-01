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

#include <libaegis/introspector/time.h>


introspector_time::~introspector_time()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


introspector_time::introspector_time(const adapter::pointer &a_how) :
    how(a_how)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


introspector_time::pointer
introspector_time::create(const adapter::pointer &a_how)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new introspector_time(a_how));
}


void
introspector_time::integer(long n)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    how->set(time_t(n));
}


void
introspector_time::real(double n)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    // Precision may be lost.
    how->set(time_t(n));
}


nstring
introspector_time::get_name()
    const
{
    return "time";
}
