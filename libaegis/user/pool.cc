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

#include <libaegis/user.h>


//
// If this was a list<user_ty::pointer> then the global destructor could
// clean up after us, reducing memory leak reports from valgrind..
//
size_t user_ty::pool_nusers_max;
size_t user_ty::pool_nusers;
user_ty::pointer *user_ty::pool_user;


user_ty::pointer
user_ty::pool_find(int uid)
{
    for (size_t j = 0; j < pool_nusers; ++j)
    {
	user_ty::pointer up = pool_user[j];
	if (up->get_uid() == uid)
            return up;
    }
    return user_ty::pointer();
}


void
user_ty::pool_add(user_ty::pointer up)
{
    if (pool_nusers >= pool_nusers_max)
    {
	pool_nusers_max = pool_nusers_max * 2 + 4;
	user_ty::pointer *new_user = new user_ty::pointer [pool_nusers_max];
	for (size_t k = 0; k < pool_nusers; ++k)
	    new_user[k] = pool_user[k];
	delete [] pool_user;
	pool_user = new_user;
    }
    pool_user[pool_nusers++] = up;
}
