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

#include <common/trace.h>

#include <libaegis/sub/functor/variable.h>


sub_functor_variable::~sub_functor_variable()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


sub_functor_variable::sub_functor_variable(const nstring &a_name,
        const nstring &a_value) :
    sub_functor(a_name),
    value(a_value),
    used(false),
    append_if_unused_flag(false),
    override_flag(false),
    resubstitute_flag(false)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
}


sub_functor::pointer
sub_functor_variable::create(const nstring &a_name, const nstring &a_value)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new sub_functor_variable(a_name, a_value));
}


wstring
sub_functor_variable::evaluate(sub_context_ty *, const wstring_list &)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    used = true;
    return value;
}


bool
sub_functor_variable::append_if_unused()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return append_if_unused_flag;
}


void
sub_functor_variable::append_if_unused_set()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    append_if_unused_flag = true;
}


bool
sub_functor_variable::override()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return override_flag;
}


void
sub_functor_variable::override_set()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    override_flag = true;
}


bool
sub_functor_variable::resubstitute()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return resubstitute_flag;
}


void
sub_functor_variable::resubstitute_set()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    resubstitute_flag = true;
}


bool
sub_functor_variable::must_be_used()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return !used;
}


void
sub_functor_variable::optional_set()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    used = true;
}
