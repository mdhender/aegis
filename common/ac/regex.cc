//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the ac_regex class
//

#include <ac/regex.h>
#include <ac/string.h>

#if !((HAVE_RXPOSIX_H && HAVE_LIBRX) || HAVE_REGEX_H)

int
regcomp(regex_t *preg, const char *regex, int cflags)
{
    return -1;
}


int
regexec(const  regex_t  *preg,  const  char *string, size_t nmatch,
    regmatch_t pmatch[], int eflags)
{
    return REG_NOMATCH;
}


size_t
regerror(int errcode, const regex_t *preg, char *errbuf,  size_t
    errbuf_size)
{
    strlcpy(errbuf, "Regular expressions not available", errbuf_size);
}


void
regfree(regex_t *preg)
{
}


#endif
