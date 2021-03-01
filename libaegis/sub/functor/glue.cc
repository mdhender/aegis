//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <libaegis/sub/functor/glue.h>


sub_functor_glue::~sub_functor_glue()
{
}


sub_functor_glue::sub_functor_glue(const nstring &a_name, func_p a_what,
        bool a_resub) :
    sub_functor(a_name),
    what(a_what),
    resub(a_resub)
{
}


sub_functor::pointer
sub_functor_glue::create(const nstring &a_name, func_p a_what)
{
    return pointer(new sub_functor_glue(a_name, a_what, false));
}


sub_functor::pointer
sub_functor_glue::create(const nstring &a_name, func_p a_what, bool a_resub)
{
    return pointer(new sub_functor_glue(a_name, a_what, a_resub));
}


wstring
sub_functor_glue::evaluate(sub_context_ty *scp, const wstring_list &args)
{
    return what(scp, args);
}


bool
sub_functor_glue::resubstitute()
    const
{
    return resub;
}


void
sub_functor_glue::resubstitute_set()
{
    resub = true;
}


bool
sub_functor_glue::append_if_unused()
    const
{
    return false;
}


void
sub_functor_glue::append_if_unused_set()
{
    // Do nothing.
}


bool
sub_functor_glue::override()
    const
{
    return false;
}


void
sub_functor_glue::override_set()
{
    // Do nothing.
}


bool
sub_functor_glue::must_be_used()
    const
{
    return false;
}


void
sub_functor_glue::optional_set()
{
    // Do nothing.
}
