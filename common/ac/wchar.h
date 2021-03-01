//
//	aegis - project change supervisor
//	Copyright (C) 1996-1999, 2001, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_WCHAR_H
#define COMMON_AC_WCHAR_H

#include <common/ac/stdarg.h>
#include <common/ac/stddef.h>

#if HAVE_WCHAR_H
#include <wchar.h>

#if !HAVE_WINT_T
#define HAVE_WINT_T 1
#ifndef _WINT_T
#define _WINT_T
typedef wchar_t wint_t;
#endif
#endif

#if !HAVE_MBSTATE_T
#define HAVE_MBSTATE_T 1
#ifndef _MBSTATE_T
#define _MBSTATE_T
typedef int mbstate_t;
#endif
#endif

#else

#include <common/ac/stddef.h>
#include <common/main.h>
typedef int mbstate_t;
#ifndef WEOF
#define WEOF (wchar_t)(-1);
#endif
int mbsinit(const mbstate_t *);
size_t wcslen(const wchar_t *);
size_t mbrlen(const char *, size_t, mbstate_t *);
size_t mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);
size_t wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);

#endif


#if !HAVE_DECL_WCWIDTH
extern "C" {
int wcwidth(wchar_t);
}
#endif

#if !HAVE_DECL_WCSWIDTH
extern "C" {
int wcswidth(const wchar_t *, size_t);
}
#endif

#if !HAVE_DECL_MBRTOWC
extern "C" {
size_t mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
}
#endif

#if !HAVE_DECL_WCRTOMB
extern "C" {
size_t wcrtomb(char *, wchar_t, mbstate_t *);
}
#endif

// Solaris bug 1250837: include wchar.h before widec.h
#if HAVE_WIDEC_H
#include <widec.h>
#endif


//
// HAVE_ISWPRINT is only set if (a) there is an iswprint function,
// and (b) it works for ascii.  It is assumed that if iswprint is absent
// or brain-dead, then so are the rest.
//
// This code copes with the case where (a) it exists, (b) it is broken,
// and (c) it is defined in <wchar.h>, of all places!
//
#if HAVE_ISWPRINT

#ifdef iswprint
#undef iswprint
#endif

#ifdef iswspace
#undef iswspace
#endif

#ifdef iswpunct
#undef iswpunct
#endif

#ifdef iswupper
#undef iswupper
#endif

#ifdef iswlower
#undef iswlower
#endif

#ifdef iswdigit
#undef iswdigit
#endif

#ifdef iswalnum
#undef iswalnum
#endif

#ifdef towupper
#undef towupper
#endif

#ifdef towlower
#undef towlower
#endif

#endif // !HAVE_ISWPRINT

//
// The ANSI C standard states that wint_t symbol shall be defined by
// <wchar.h> and <wctype.h>.  The GNU people also define it in <stddef.h>,
// but this is incorrect.
//
#ifndef HAVE_WINT_T
#define HAVE_WINT_T
typedef wchar_t wint_t;
#endif

#endif // COMMON_AC_WCHAR_H
