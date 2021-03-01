//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 2002-2004, 2006, 2008 Peter Miller.
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

#ifndef MAIN_H
#define MAIN_H

/** \addtogroup Common
  * \brief A common set of functions and types
  *
  */

//
// Generate a syntax error for all C++ keywords that aren't C keywords.
// This ensures that we won't write C code that can't be complied by a
// C++ compiler in the future.
//
#ifndef __cplusplus
#define bool		/!/!/
#define catch		/!/!/
#define class		/!/!/
#define complex		/!/!/
#define const_cast	/!/!/
#define delete		/!/!/
#define dynamic_cast	/!/!/
#define explicit	/!/!/
#define false		/!/!/
#define friend		/!/!/
#define inline		/!/!/
#define mutable		/!/!/
#define namespace	/!/!/
#define new		/!/!/
#define operator	/!/!/
#define private		/!/!/
#define protected	/!/!/
#define public		/!/!/
#define reinterpret_cast /!/!/
#define static_cast	/!/!/
#define template	/!/!/
#define this		/!/!/
#define throw		/!/!/
#define true		/!/!/
#define try		/!/!/
#define typeid		/!/!/
#define typename	/!/!/
#define using		/!/!/
#define virtual		/!/!/
#endif

//
// array manipulation
//
#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))
#define ENDOF(a) ((a) + SIZEOF(a))

//
// Function attributes.
//
#ifdef __GNUC__
#define ATTR_PRINTF(x, y) __attribute__((format(printf, x, y)))
#define ATTR_VPRINTF(x) __attribute__((format(printf, x, 0)))
#define NORETURN __attribute__((noreturn))
#else
#define ATTR_PRINTF(x, y)
#define ATTR_VPRINTF(x)
#define NORETURN
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define DEPRECATED __attribute__((deprecated))
#else
#define DEPRECATED
#endif

//
// Take the define out of comments to
// enable the debugging the functionality.
//
// #define DEBUG
//

#endif // MAIN_H
