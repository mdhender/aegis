//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_ZLIB_H
#define COMMON_AC_ZLIB_H

#include <common/config.h>

#if HAVE_ZLIB_H
#include <zlib.h>
#else
#error "You must have zlib installed in order to build Aegis."
#endif

// there is ERR_MSG in zutil.h, but we aren't allowed to see it.
#include <common/main.h>
const char *z_error(int);

#ifndef DEF_MEM_LEVEL
#define DEF_MEM_LEVEL 8 // in zutil.h, but we aren't allowed to see it.
#endif

#endif // COMMON_AC_ZLIB_H
