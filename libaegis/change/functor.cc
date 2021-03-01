//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the change_functor class
//

#pragma implementation "change_functor"

#include <change/functor.h>


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
