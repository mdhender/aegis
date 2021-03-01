//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2012 Peter Miller
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

#include <common/trace.h>

#include <libaegis/sub/functor.h>


sub_functor::~sub_functor()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


sub_functor::sub_functor(const nstring &a_name) :
    name(a_name)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


bool
sub_functor::append_if_unused()
    const
{
    return false;
}


bool
sub_functor::override()
    const
{
    return false;
}


bool
sub_functor::resubstitute()
    const
{
    return false;
}


bool
sub_functor::must_be_used()
    const
{
    return false;
}


void
sub_functor::resubstitute_set()
{
    // Do nothing.
}


void
sub_functor::override_set()
{
    // Do nothing.
}


void
sub_functor::optional_set()
{
    // Do nothing.
}


void
sub_functor::append_if_unused_set()
{
    // Do nothing.
}


// vim: set ts=8 sw=4 et :
