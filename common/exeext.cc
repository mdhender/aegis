//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004-2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <common/exeext.h>
#include <common/libdir.h>


static int
memcasecmp(const char *s1, const char *s2, size_t n)
{
    unsigned char   c1;
    unsigned char   c2;

    while (n > 0)
    {
	c1 = *s1++;
	if (isupper((unsigned char)c1))
	    c1 = tolower(c1);
	c2 = *s2++;
	if (isupper((unsigned char)c2))
	    c1 = tolower(c2);
	if (c1 != c2)
	    return (c1 - c2);
	--n;
    }
    return 0;
}


static int
look_for_suffix(const char *stem, const char *suffix)
{
    size_t	    stem_len;
    size_t	    suffix_len;
    size_t	    idx;

    stem_len = strlen(stem);
    suffix_len = strlen(suffix);
    if (stem_len < suffix_len)
	return -1;
    idx = stem_len - suffix_len;
    if (0 != memcasecmp(stem + idx, suffix, suffix_len))
	return -1;
    return idx;
}


int
exeext(const char *s)
{
    const char	*cfg;
    int		n;

    cfg = configured_exeext();
    if (!*cfg)
	return -1;
    n = look_for_suffix(s, cfg);
    if (n >= 0)
	return n;
#if defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(__NUTC__)
    n = look_for_suffix(s, ".exe");
    if (n >= 0)
	return n;
    n = look_for_suffix(s, ".bat");
    if (n >= 0)
	return n;
    n = look_for_suffix(s, ".cmd");
    if (n >= 0)
	return n;
    n = look_for_suffix(s, ".com");
    if (n >= 0)
	return n;
#endif
    return -1;
}
