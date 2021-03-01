//
//	aegis - a project change supervisor
//	Copyright (C) 1994, 1996, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef COMMON_AC_DIRENT_H
#define COMMON_AC_DIRENT_H

#include <common/ac/unistd.h>

// unistd.h defines _POSIX_VERSION on POSIX.1 systems.
#if defined(DIRENT) || defined(_POSIX_VERSION)
#include <dirent.h>
#define NLENGTH(dirent) (strlen((dirent)->d_name))
#else // not (DIRENT or _POSIX_VERSION)
#define dirent direct
#define NLENGTH(dirent) ((dirent)->d_namlen)
#ifdef SYSNDIR
#include <sys/ndir.h>
#endif // SYSNDIR
#ifdef SYSDIR
#include <sys/dir.h>
#endif // SYSDIR
#ifdef NDIR
#include <ndir.h>
#endif // NDIR
#endif // not (DIRENT or _POSIX_VERSION)

#endif // COMMON_AC_DIRENT_H
