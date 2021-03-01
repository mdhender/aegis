/*
 *	aegis - a project change supervisor
 *	Copyright (C) 1994, 1996, 1997, 2002, 2004 Peter Miller;
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
 * MANIFEST: insulate against <string.h> vs <strings.h> differences
 */

#ifndef COMMON_AC_STRING_H
#define COMMON_AC_STRING_H

#include <config.h>

#if !HAVE_STRCASECMP
int strcasecmp(const char *, const char *);
#endif

#if !HAVE_STRNCASECMP
int strncasecmp(const char *, const char *, size_t);
#endif

#if !HAVE_STRSIGNAL
char *strsignal(int);
#endif

#if !HAVE_STRVERSCMP
int strverscmp(const char *, const char *);
#endif

#if STDC_HEADERS || HAVE_STRING_H
#  include <string.h>
   /* An ANSI string.h and pre-ANSI memory.h might conflict.  */
#  if !STDC_HEADERS && HAVE_MEMORY_H
#    include <memory.h>
#  endif
#else
   /* memory.h and strings.h conflict on some systems.  */
#  include <strings.h>
#endif

#endif /* COMMON_AC_STRING_H */
