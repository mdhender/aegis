/*
 *	aegis - project change supervisor
 *	Copyright (C) 1998 Peter Miller;
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
 * MANIFEST: interface definition for config.messy.c
 *
 * This file is included by the generated ``common/config.h'' file.
 * These actions are performed ehre, to insulate them from the attentions
 * of ./configure (config.status) which is a little over-0zealous about
 * nuking the #undef lines.
 */

#ifndef COMMON_CONFIG_MESSY_H
#define COMMON_CONFIG_MESSY_H

/*
 * Define this symbol if your system does NOT
 * have the seteuid system call, and it cannot be simulated.
 */
#ifndef HAVE_SETEUID
#ifndef HAVE_SETREUID
#ifndef HAVE_SETRESUID
#define CONF_NO_seteuid
#endif
#endif
#endif

/*
 * Make sure Solaris includes POSIX extensions.
 */
#if (defined(__sun) || defined(__sun__) || defined(sun)) && \
	(defined(__svr4__) || defined(svr4))

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif

/*
 * fix a glitch in Solaris's <sys/time.h>
 * which only show's up when you turn __EXTENSIONS__ on
 */
#define _timespec timespec      /* fix 2.4 */
#define _tv_sec tv_sec          /* fix 2.5.1 */

#endif /* Solaris */

/*
 * normalize the wide character support
 */
#if defined(HAVE_WCTYPE_H) && !defined(HAVE_ISWPRINT)
# undef HAVE_WCTYPE_H
# ifdef HAVE_ISWCTYPE
#  undef HAVE_ISWCTYPE
# endif
#endif
#if !defined(HAVE_WCTYPE_H) && defined(HAVE_ISWPRINT)
# undef HAVE_ISWPRINT
#endif
#if defined(HAVE_WIDEC_H) && !defined(HAVE_WCTYPE_H)
# undef HAVE_WIDEC_H
#endif

#endif /* COMMON_CONFIG_MESSY_H */