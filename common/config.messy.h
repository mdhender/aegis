//
//      aegis - project change supervisor
//      Copyright (C) 1998, 1999, 2001-2006, 2008, 2010, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//
// This file is included by the generated "common/config.h" file.
// These actions are performed ehre, to insulate them from the attentions
// of ./configure (config.status) which is a little over-0zealous about
// nuking the #undef lines.
//

#ifndef COMMON_CONFIG_MESSY_H
#define COMMON_CONFIG_MESSY_H

//
// Define this symbol if your system does NOT
// have the seteuid system call, and it cannot be simulated.
//
#ifndef HAVE_SETEUID
#ifndef HAVE_SETREUID
#ifndef HAVE_SETRESUID
#define CONF_NO_seteuid
#endif
#endif
#endif

//
// For libcurl to work, it must be able to run in the top process,
// not forked off in a setuid worker process.  (This isn't a problem
// for most modern systems.)
//
#ifdef CONF_NO_seteuid
#undef HAVE_CURL_CURL_H
#undef HAVE_LIBCURL
#endif

//
// Make sure Solaris includes POSIX extensions.
//
#if (defined(__sun) || defined(__sun__) || defined(sun)) && \
        (defined(__svr4__) || defined(svr4))

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#ifndef __EXTENSIONS__
#define __EXTENSIONS__ 1
#endif

//
// fix a glitch in Solaris's <sys/time.h>
// which only show's up when you turn __EXTENSIONS__ on
//
#define _timespec timespec      // fix 2.4
#define _tv_sec tv_sec          // fix 2.5.1

#endif // Solaris

//
// normalize the wide character support
//
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

//
// Need to define _POSIX_SOURCE on Linux, in order to get the fdopen,
// fileno, popen and pclose function prototypes.
//
#ifdef __linux__
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#endif

//
// The SINGLE_USER define may be exploited to create a single-user version
// of Aegis.  It is mostly used for testing, but also for Windows NT,
// as the security there is stuffed.
//
#if defined(SOURCE_FORGE_HACK) ||  defined(__CYGWIN__) || \
        defined(__CYGWIN32__) || defined(__NUTC__)
#define SINGLE_USER
#endif

//
// Cope with parts of libmagic being missing
// (or being something else, e.g. on HP/UX)
//
#if !HAVE_MAGIC_H || !HAVE_LIBMAGIC || !HAVE_MAGIC_FILE
# ifdef HAVE_MAGIC_H
#  undef HAVE_MAGIC_H
# endif
# ifdef HAVE_LIBMAGIC
#  undef HAVE_LIBMAGIC
# endif
# ifdef HAVE_MAGIC_FILE
#  undef HAVE_MAGIC_FILE
# endif
#endif

//
// The configure script will set UUID_OK to zero if it can't find a
// working combination from all of the include files and functions it
// found.  Unfortunately, due to the way autoconf works, it is to late
// to cancel the various defines about all of those searches, so we do
// it here.
//
#if ! UUID_OK
#undef HAVE_DCE_UUID_H
#undef HAVE_SYS_UUID_H
#undef HAVE_UUID_CREATE
#undef HAVE_UUID_EXPORT
#undef HAVE_UUID_GENERATE
#undef HAVE_UUID_H
#undef HAVE_UUID_HASH
#undef HAVE_UUID_LOAD
#undef HAVE_UUID_MAKE
#undef HAVE_UUID_TO_STRING
#undef HAVE_UUID_UNPARSE
#undef HAVE_UUID_UUID_H
#endif

//
// The OFF_T_FMT is used to print value of the off_t type.  We use the
// standard %lld format for long long value.
//
#if _FILE_OFFSET_BITS == 64 && LONG_BIT < 64
#define OFF_T_FMT   "%lld"
#else
#define OFF_T_FMT   "%ld"
#endif


#endif // COMMON_CONFIG_MESSY_H
// vim: set ts=8 sw=4 et :
