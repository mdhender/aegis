//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 1995, 1999, 2002, 2004-2008 Peter Miller
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

#ifndef ERROR_H
#define ERROR_H

#include <common/main.h>

/** \addtogroup Error
  * \brief Error functions
  * \ingroup Common
  * @{
  */
void error_raw(const char *, ...);
void fatal_raw(const char *, ...) NORETURN;

void nerror(const char *, ...);
void nfatal(const char *, ...) NORETURN;

void assert_failed(const char *condition, const char *file, int line)
    NORETURN;
#ifndef assert
# ifdef DEBUG
#  define assert(c) ((c) ? (void)0 : assert_failed(#c, __FILE__, __LINE__))
# else
#  define assert(c)
# endif
#endif

#define this_is_a_bug() \
	fatal_raw							\
	(								\
"you have found a bug (file %s, line %d) please report it immediately", \
		__FILE__,						\
		__LINE__						\
	)

/** @} */
#endif // ERROR_H
