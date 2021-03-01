//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2004-2006, 2008 Peter Miller
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

#include <common/ac/unistd.h>


#ifndef CONF_NO_seteuid
#ifndef HAVE_SETEUID
#ifdef HAVE_SETRESUID

int
seteuid(int x)
{
    return setresuid(-1, x, -1);
}


int
setegid(int x)
{
    return setresgid(-1, x, -1);
}


#else
#ifdef HAVE_SETREUID


int
seteuid(int x)
{
    return setreuid(-1, x);
}


int
setegid(int x)
{
    return setregid(-1, x);
}


#endif // HAVE_SETREUID
#endif // !HAVE_SETRESUID
#endif // !HAVE_SETEUID
#endif // !CONF_NO_seteuid
