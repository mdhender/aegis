//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1996, 1998, 1999, 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_LIBINTL_H
#define COMMON_AC_LIBINTL_H

#include <common/config.h>

//
// if the libintl.h include file is available, include it
//
#if HAVE_LIBINTL_H

//
// On some platforms (particularly those which don't have a native GNU
// libc) you need to define the ENABLE_NLS, or nothing will happen.
//
#define ENABLE_NLS 1

#include <libintl.h>
#else
#if HAVE_LIBGETTEXT_H
#include <libgettext.h>
#else

//
// otherwise, provide a prototype and nothing else
//
char *gettext(const char *);
#endif // !HAVE_LIBGETTEXT_H
#endif // !HAVE_LIBINTL_H

#endif // COMMON_AC_LIBINTL_H
