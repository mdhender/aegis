//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/ac/regex.h>
#include <common/ac/string.h>

#if !((HAVE_RXPOSIX_H && HAVE_LIBRX) || HAVE_REGEX_H)

int
regcomp(regex_t *, const char *, int)
{
    return -1;
}


int
regexec(const regex_t *, const char *, size_t, regmatch_t [], int)
{
    return REG_NOMATCH;
}


size_t
regerror(int, const regex_t *, char *errbuf,  size_t errbuf_size)
{
    strendcpy
    (
	errbuf,
	"Regular expressions not available",
	errbuf + errbuf_size
    );
    return strlen(errbuf);
}


void
regfree(regex_t *)
{
}


#endif
