//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//

#ifndef LIBAEGIS_SAFE_FREOPEN_H
#define LIBAEGIS_SAFE_FREOPEN_H

#include <common/ac/stdio.h>


/**
  * The safe_freopen function is an error reporting wrapper around the
  * freopen system library method.
  */
void safe_freopen(const char *path, const char *mode, FILE *fp);

#endif // LIBAEGIS_SAFE_FREOPEN_H
