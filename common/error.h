/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 1995, 1999, 2002, 2004 Peter Miller;
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
 * MANIFEST: interface definition for common/error.c
 */

#ifndef ERROR_H
#define ERROR_H

#include <main.h>

/** \addtogroup Error
  * \brief Error functions
  * \ingroup Common
  * @{
  */
#define error /!/!/
void error_raw(const char *, ...);
#define fatal /!/!/
void fatal_raw(const char *, ...) NORETURN;

void nerror(const char *, ...);
void nfatal(const char *, ...) NORETURN;

void assert_failed(const char *condition, const char *file, int line)
    NORETURN;
#ifdef DEBUG
# define assert(c) ((c) ? (void)0 : assert_failed(#c, __FILE__, __LINE__))
#else
# define assert(c)
#endif

#define this_is_a_bug() \
	fatal_raw							\
	(								\
"you have found a bug (file %s, line %d) please report it immediately", \
		__FILE__,						\
		__LINE__						\
	)

/** @} */
#endif /* ERROR_H */
