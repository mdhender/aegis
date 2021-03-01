//
//	aegis - project change supervisor
//	Copyright (C) 1998-2000, 2004-2006, 2008 Peter Miller
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

#ifndef COMMON_AC_REGEX_H
#define COMMON_AC_REGEX_H

//
// The rxposix.h and regex.h include files need size_t.  We make this
// file indempotent, so that code which includes *this* file doesn't
// need to worry about the include file ordering.
//
#include <common/ac/stddef.h>
#include <common/ac/sys/types.h>

#if HAVE_RXPOSIX_H && HAVE_LIBRX
extern "C" {
#include <rxposix.h>
}
#else
#if HAVE_REGEX_H

//
// The GNU Rx library has a broken usage of __restrict,
// and we need to make sure it doesn't foul the compiler.
//
#undef __restrict_arr
#define __restrict_arr
#include <regex.h>
#undef __restrict_arr
#else

//
// Fake just enough to get things to compile.
//
#define regex_t int

struct regmatch_t
{
    int rm_so;
    int rm_eo;
};

#define REG_EXTENDED 0
#define REG_NOSUB 0
#define REG_NOMATCH -1
#define REG_NOTBOL 0

int regcomp(regex_t *preg, const char *regex, int cflags);
int regexec(const regex_t *preg, const char *string, size_t nmatch,
    regmatch_t *pmatch, int eflags);
size_t regerror(int errcode, const regex_t *preg, char *errbuf,
    size_t errbuf_size);
void regfree(regex_t *preg);

#endif
#endif

#endif // COMMON_AC_REGEX_H
