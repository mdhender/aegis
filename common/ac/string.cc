//
//      aegis - project change supervisor
//      Copyright (C) 1994, 1996, 2002-2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/signal.h>


//
//  NAME
//      strerror - string for error number
//
//  SYNOPSIS
//      char *strerror(int errnum);
//
//  DESCRIPTION
//      The strerror function maps the error number in errnum to an error
//      message string.
//
//  RETURNS
//      The strerror function returns a pointer to the string, the contents of
//      which are implementation-defined.  The array pointed to shall not be
//      modified by the program, but may be overwritten by a subsequent call to
//      the strerror function.
//
//  CAVEAT
//      Unknown errors will be rendered in the form "Error %d", where %d will
//      be replaced by a decimal representation of the error number.
//

#ifndef HAVE_STRERROR

char *
strerror(int n)
{
    extern int  sys_nerr;
    extern const char *const sys_errlist[];
    static char buffer[16];

    if (n < 1 || n > sys_nerr)
    {
        snprintf(buffer, sizeof(buffer), "Error %d", n);
        return buffer;
    }
    return (char *)sys_errlist[n];
}

#endif // !HAVE_STRERROR


#ifndef HAVE_STRCASECMP

#include <common/ac/ctype.h>

int
strcasecmp(const char *s1, const char *s2)
{
    unsigned char   c1;
    unsigned char   c2;

    for (;;)
    {
        c1 = *s1++;
        if (isupper((unsigned char)c1))
            c1 = tolower(c1);
        c2 = *s2++;
        if (isupper((unsigned char)c2))
            c2 = tolower(c2);
        if (c1 != c2)
        {
            //
            // if s1 is a leading substring of s2, must
            // return -1, even if the next character of s2
            // is negative.
            //
            if (!c1)
                return -1;
            if (!c2)
                return 1;
            if (c1 < c2)
                return -1;
            return 1;
        }
        if (!c1)
            return 0;
    }
}

#endif // !HAVE_STRCASECMP


#ifndef HAVE_STRNCASECMP

#include <common/ac/ctype.h>

int
strncasecmp(const char *s1, const char *s2, size_t len)
{
    unsigned char   c1;
    unsigned char   c2;

    while (len-- > 0)
    {
        c1 = *s1++;
        if (isupper((unsigned char)c1))
            c1 = tolower(c1);
        c2 = *s2++;
        if (isupper((unsigned char)c2))
            c2 = tolower(c2);
        if (c1 != c2)
        {
            //
            // if s1 is a leading substring of s2, must
            // return -1, even if the next character of s2
            // is negative.
            //
            if (!c1)
                return -1;
            if (!c2)
                return 1;
            if (c1 < c2)
                return -1;
            return 1;
        }
        if (!c1)
            return 0;
    }
    return 0;
}

#endif // !HAVE_STRNCASECMP


#ifndef HAVE_STRSIGNAL

extern "C" const char *
strsignal(int n)
{
    switch (n)
    {
#ifdef SIGHUP
    case SIGHUP:
        return "hang up [SIGHUP]";
#endif // SIGHUP

#ifdef SIGINT
    case SIGINT:
        return "user interrupt [SIGINT]";
#endif // SIGINT

#ifdef SIGQUIT
    case SIGQUIT:
        return "user quit [SIGQUIT]";
#endif // SIGQUIT

#ifdef SIGILL
    case SIGILL:
        return "illegal instruction [SIGILL]";
#endif // SIGILL

#ifdef SIGTRAP
    case SIGTRAP:
        return "trace trap [SIGTRAP]";
#endif // SIGTRAP

#ifdef SIGIOT
    case SIGIOT:
        return "abort [SIGIOT]";
#endif // SIGIOT

#ifdef SIGEMT
    case SIGEMT:
        return "EMT instruction [SIGEMT]";
#endif // SIGEMT

#ifdef SIGFPE
    case SIGFPE:
        return "floating point exception [SIGFPE]";
#endif // SIGFPE

#ifdef SIGKILL
    case SIGKILL:
        return "kill [SIGKILL]";
#endif // SIGKILL

#ifdef SIGBUS
    case SIGBUS:
        return "bus error [SIGBUS]";
#endif // SIGBUS

#ifdef SIGSEGV
    case SIGSEGV:
        return "segmentation violation [SIGSEGV]";
#endif // SIGSEGV

#ifdef SIGSYS
    case SIGSYS:
        return "bad argument to system call [SIGSYS]";
#endif // SIGSYS

#ifdef SIGPIPE
    case SIGPIPE:
        return "write on a pipe with no one to read it [SIGPIPE]";
#endif // SIGPIPE

#ifdef SIGALRM
    case SIGALRM:
        return "alarm clock [SIGALRM]";
#endif // SIGALRM

#ifdef SIGTERM
    case SIGTERM:
        return "software termination [SIGTERM]";
#endif // SIGTERM

#ifdef SIGUSR1
    case SIGUSR1:
        return "user defined signal one [SIGUSR1]";
#endif // SIGUSR1

#ifdef SIGUSR2
    case SIGUSR2:
        return "user defined signal two [SIGUSR2]";
#endif // SIGUSR2

#ifdef SIGCLD
    case SIGCLD:
        return "death of child [SIGCLD]";
#endif // SIGCLD

#ifdef SIGPWR
    case SIGPWR:
        return "power failure [SIGPWR]";
#endif // SIGPWR
    }
    return 0;
}

#endif // !HAVE_STRSIGNAL

#ifndef HAVE_STRVERSCMP

//
// Derived from sources marked...
//
// Compare strings while treating digits characters numerically.
// Copyright (C) 1997 Free Software Foundation, Inc.
// This file is part of the GNU C Library.
// Contributed by Jean-Franc,ois Bignolles <bignolle@ecoledoc.ibp.fr>, 1997.
//
// The GNU C Library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// The GNU C Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with the GNU C Library; see the file COPYING.LIB.  If not,
// write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
// Floor, Boston, MA 02110-1301 USA.
//

#include <common/ac/ctype.h>

//
// states:
// S_N: normal,
// S_I: comparing integral part,
// S_F: comparing fractionnal parts,
// S_Z: idem but with leading Zeroes only
//
#define  S_N    0x0
#define  S_I    0x4
#define  S_F    0x8
#define  S_Z    0xC

//
// result_type:
// CMP: return diff;
// LEN: compare using len_diff/diff
//
#define  CMP    2
#define  LEN    3


//
// Compare S1 and S2 as strings holding indices/version numbers, returning
// less than, equal to or greater than zero if S1 is less than, equal
// to or greater than S2 (for more info, see the texinfo doc).
//

int
strverscmp(const char *s1, const char *s2)
{
    const unsigned char *p1;
    const unsigned char *p2;
    unsigned char   c1;
    unsigned char   c2;
    int             state;
    int             diff;

    // Symbol(s)    others  [0-9]        0  (padding)
    // Transition   (00) x  (01) d  (10) 0  (11) -
    static const unsigned int next_state[] =
    {
        // state         x       d       0       -
        // S_N
                        S_N,    S_I,    S_Z,    S_N,
        // S_I
                        S_N,    S_I,    S_I,    S_I,
        // S_F
                        S_N,    S_F,    S_F,    S_F,
        // S_Z
                        S_N,    S_F,    S_Z,    S_Z
    };

    static const int result_type[] =
    {
        // state   x/x  x/d  x/0  x/-  d/x  d/d  d/0  d/-
        //         0/x  0/d  0/0  0/-  -/x  -/d  -/0  -/-
        // S_N
                   CMP, CMP, CMP, CMP, CMP, LEN, CMP, CMP,
                   CMP, CMP, CMP, CMP, CMP, CMP, CMP, CMP,
        // S_I
                   CMP,  -1,  -1, CMP,  +1, LEN, LEN, CMP,
                    +1, LEN, LEN, CMP, CMP, CMP, CMP, CMP,
        // S_F
                   CMP, CMP, CMP, CMP, CMP, LEN, CMP, CMP,
                   CMP, CMP, CMP, CMP, CMP, CMP, CMP, CMP,
        // S_Z
                   CMP,  +1,  +1, CMP,  -1, CMP, CMP, CMP,
                    -1, CMP, CMP, CMP,   0,   0,   0,   0,
    };

    p1 = (const unsigned char *)s1;
    p2 = (const unsigned char *)s2;
    if (p1 == p2)
        return 0;
    c1 = *p1++;
    c2 = *p2++;
    // Hint: '0' is a digit too.
    state = S_N | ((c1 == '0') + (isdigit(c1) != 0));
    while ((diff = c1 - c2) == 0 && c1 != '\0')
    {
        state = next_state[state];
        c1 = *p1++;
        c2 = *p2++;
        state |= (c1 == '0') + (isdigit(c1) != 0);
    }
    state = result_type[state << 2 | (((c2 == '0') + (isdigit(c2) != 0)))];
    switch (state)
    {
    case CMP:
        return diff;

    case LEN:
        while (isdigit(*p1++))
        {
            if (!isdigit(*p2++))
                return 1;
        }
        return isdigit(*p2) ? -1 : diff;

    default:
        return state;
    }
}

#endif // !HAVE_STRVERSCMP
#if !HAVE_STRLCPY

//
// http://www.courtesan.com/todd/papers/strlcpy.html
//
// Like strncpy but does not 0 fill the buffer and always null
// terminates.
//
// @param bufsize is the size of the destination buffer.
//
// @return index of the terminating byte.
//
size_t
strlcpy(char *d, const char *s, size_t bufsize)
{
    if (bufsize <= 0)
        return 0;
    size_t len = strlen(s);
    size_t ret = len;
    if (len >= bufsize)
        len = bufsize - 1;
    memcpy(d, s, len);
    d[len] = 0;
    return ret;
}

#endif // !HAVE_STRLCPY
#if !HAVE_STRLCAT

//
// http://www.courtesan.com/todd/papers/strlcpy.html
//
// Like strncat() but does not 0 fill the buffer and always null
// terminates.
//
// @param bufsize length of the buffer, which should be one more than
// the maximum resulting string length.
//
size_t
strlcat(char *d, const char *s, size_t bufsize)
{
    size_t len1 = strlen(d);
    size_t len2 = strlen(s);
    size_t ret = len1 + len2;
    if (len1 + len2 >= bufsize)
        len2 = bufsize - len1 - 1;
    if (len2 > 0)
    {
        memcpy(d + len1, s, len2);
        d[len1 + len2] = 0;
    }
    return ret;
}

#endif // !HAVE_STRLCAT
#ifndef HAVE_STRENDCPY

char *
strendcpy(char *dst, const char *src, const char *end)
{
    if (dst < end)
    {
        // leave room for terminating NUL
        end--;
        while (dst < end && *src)
            *dst++ = *src++;
        *dst = '\0';
        if (*src)
        {
            // return end parameter if truncated
            dst++;
        }
    }
    return dst;
}

#endif // !HAVE_STRENDCPY
