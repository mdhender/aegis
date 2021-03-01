/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate exeexts
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <exeext.h>
#include <libdir.h>


static int memcasecmp _((const char *, const char *, size_t));

static int
memcasecmp(s1, s2, n)
	const char	*s1;
	const char	*s2;
	size_t		n;
{
	int		c1, c2;

	while (n > 0)
	{
		c1 = *s1++;
		if (isupper(c1))
			c1 = tolower(c1);
		c2 = *s2++;
		if (isupper(c2))
			c1 = tolower(c2);
		if (c1 != c2)
			return ((unsigned char)c1 - (unsigned char)c2);
		--n;
	}
	return 0;
}


static int look_for_suffix _((const char *, const char *));

static int
look_for_suffix(main, suffix)
	const char	*main;
	const char	*suffix;
{
	size_t		main_len;
	size_t		suffix_len;
	size_t		idx;

	main_len = strlen(main);
	suffix_len = strlen(suffix);
	if (main_len < suffix_len)
		return -1;
	idx = main_len - suffix_len;
	if (0 != memcasecmp(main + idx, suffix, suffix_len))
		return -1;
	return idx;
}


int
exeext(s)
	const char	*s;
{
	const char	*cfg;
	int		n;

	cfg = configured_exeext();
	if (!*cfg)
		return -1;
	n = look_for_suffix(s, cfg);
	if (n >= 0)
		return n;
#if defined(__CYGWIN__) || defined(__CYGWIN32__)
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
