//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/change/functor.h>
#include <common/now.h>


change_functor::~change_functor()
{
    // Nothing to do.
}


change_functor::change_functor(bool arg1, bool arg2) :
    include_branches_flag(arg1),
    all_changes_flag(arg2)
{
}


change_functor::change_functor(const change_functor &arg) :
    include_branches_flag(arg.include_branches_flag)
{
}


change_functor &
change_functor::operator=(const change_functor &arg)
{
    if (this != &arg)
    {
	include_branches_flag = arg.include_branches_flag;
    }
    return *this;
}


time_t
change_functor::earliest()
{
    return 0;
}


time_t
change_functor::latest()
{
    return now();
}


bool
change_functor::recurse_branches()
{
    return true;
}
