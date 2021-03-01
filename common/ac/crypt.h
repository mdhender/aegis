//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_CRYPT_H
#define COMMON_AC_CRYPT_H

// Sometimes crypt() is declared in <unistd.h>
// this pull sin config.h as well, for the following ifdef
#include <common/ac/unistd.h>

// Sometimes crypt() is declared in <crypt.h>
#if HAVE_CRYPT_H
#include <crypt.h>
#endif

#include <common/throws_not.h>

#if HAVE_CRYPT
#if !HAVE_DECL_CRYPT
extern "C" {
char *crypt(const char *, const char *) __THROW;
}
#endif
#endif

#endif // COMMON_AC_CRYPT_H
