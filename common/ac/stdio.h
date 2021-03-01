//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2004-2006, 2008 Peter Miller
//	Copyright (C) 2008 Walter Franzini
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

#ifndef COMMON_AC_STDIO_H
#define COMMON_AC_STDIO_H

#include <common/config.h>

#include <cstdio>

//
// The bits/c++locale.h include define an inline function that may use
// one of the symbols (re)defined below, this will lead to compilation
// on error on system's include on some platforms (eg. FreeBSD).
// In order to preserve the sprintf/vsprintf detection feature in the
// Aegis source code, we read the named include before defining the
// macro below.
//
#if HAVE_BITS_C__LOCALE_H
#include <bits/c++locale.h>
#endif

#define sprintf use_snprintf_instead
#define vsprintf use_vsnprintf_instead

#endif // COMMON_AC_STDIO_H
