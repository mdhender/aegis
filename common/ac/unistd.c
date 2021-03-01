/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1996 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: impliment missing functions from <unistd.h>
 */

#include <ac/unistd.h>


#ifndef CONF_NO_seteuid
#ifndef HAVE_SETEUID
#ifdef HAVE_SETRESUID

int
seteuid(x)
	int	x;
{
	return setresuid(-1, x, -1);
}


int
setegid(x)
	int	x;
{
	return setresgid(-1, x, -1);
}


#else
#ifdef HAVE_SETREUID


int
seteuid(x)
	int	x;
{
	return setreuid(-1, x);
}


int
setegid(x)
	int	x;
{
	return setregid(-1, x);
}


#endif /* HAVE_SETREUID */
#endif /* !HAVE_SETRESUID */
#endif /* !HAVE_SETEUID */
#endif /* !CONF_NO_seteuid */