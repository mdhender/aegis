//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#include <libaegis/dir/functor/callback.h>


dir_functor_callback::~dir_functor_callback()
{
}


dir_functor_callback::dir_functor_callback(dir_walk_callback_ty arg1,
	void *arg2) :
    callback(arg1),
    auxilliary(arg2)
{
}


void
dir_functor_callback::operator()(msg_t msg, const nstring &path,
    const struct stat &st)
{
    switch (msg)
    {
    case msg_dir_before:
	callback(auxilliary, dir_walk_dir_before, path.get_ref(), &st);
	break;

    case msg_dir_after:
	callback(auxilliary, dir_walk_dir_after, path.get_ref(), &st);
	break;

    case msg_file:
	callback(auxilliary, dir_walk_file, path.get_ref(), &st);
	break;

    case msg_special:
	callback(auxilliary, dir_walk_special, path.get_ref(), &st);
	break;

    case msg_symlink:
	callback(auxilliary, dir_walk_symlink, path.get_ref(), &st);
	break;
    }
}
