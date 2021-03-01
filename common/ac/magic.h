//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_MAGIC_H
#define COMMON_AC_MAGIC_H

#include <common/config.h>

#if HAVE_MAGIC_H
extern "C" {
#include <magic.h>
}
#else
typedef void *magic_t;
magic_t magic_open(int flags);
void magic_close(magic_t cookie);
const char *magic_file(magic_t cookie, const char *filename);
const char *magic_error(magic_t cookie);
int magic_load(magic_t cookie, const char *file_name);
#define MAGIC_MIME 0
#define MAGIC_SYMLINK 0
#endif

#endif // COMMON_AC_MAGIC_H
