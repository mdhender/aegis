/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/getpw_cache.c
 */

#ifndef LIBAEGIS_GETPW_CACHE_H
#define LIBAEGIS_GETPW_CACHE_H

#include <main.h>
#include <ac/pwd.h>

struct string_ty; /* forward */

struct passwd *getpwnam_cached _((struct string_ty *name));
struct passwd *getpwuid_cached _((int uid));

#endif /* LIBAEGIS_GETPW_CACHE_H */