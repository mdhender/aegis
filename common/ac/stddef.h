/*
 *	aegis - a project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: isolate stddef.h presence/absence
 */

#ifndef COMMON_AC_STDDEF_H
#define COMMON_AC_STDDEF_H

#include <config.h>

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifndef offsetof
#define offsetof(a, b)	((size_t)((char *)&((a *)0)->b - (char *)0))
#endif

#endif /* COMMON_AC_STDDEF_H */
