//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1995, 1999, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for common/mprintf.c
//

#ifndef COMMON_MPRINTF_H
#define COMMON_MPRINTF_H

#include <common/ac/stdarg.h>
#include <common/main.h>

/** \addtogroup Formatting
  * \brief String formatting
  * \ingroup Common
  * @{
  */

char *mprintf(const char *fmt, ...);
char *mprintf_errok(const char *fmt, ...);
char *vmprintf(const char *fmt, va_list);
char *vmprintf_errok(const char *fmt, va_list);
struct string_ty *vmprintf_str(const char *fmt, va_list);

/** @} */
#endif // COMMON_MPRINTF_H
