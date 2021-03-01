/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 2002-2004 Peter Miller.
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
 * MANIFEST: common definitions, including DEBUG define
 */

#ifndef MAIN_H
#define MAIN_H

/*
 * Generate a syntax error for all C++ keywords that aren't C keywords.
 * This ensures that we won't write C code that can't be complied by a
 * C++ compiler in the future.
 */
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

/*
 * array manipulation
 */
#define SIZEOF(a) (sizeof(a) / sizeof(a[0]))
#define ENDOF(a) ((a) + SIZEOF(a))

/*
 * Function attributes.
 */
#ifdef __GNUC__
#define ATTR_PRINTF(x, y) __attribute__((format(printf, x, y)))
#define NORETURN __attribute__((noreturn))
#else
#define ATTR_PRINTF(x, y)
#define NORETURN
#endif

/*
 * Take the define out of comments to
 * enable the debugging the functionality.
 *
#define DEBUG
 */

#endif /* MAIN_H */
