/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1998 Peter Miller;
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
 * MANIFEST: functions to manipulate abbreviates
 */

#include <ac/ctype.h>
#include <ac/string.h>

#include <abbreviate.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <str_list.h>


static string_ty *abbreviate _((string_ty *, int, int));

static string_ty *
abbreviate(s, max, keep_last_dot)
	string_ty	*s;
	int		max;
	int		keep_last_dot;
{
	string_list_ty	word;
	size_t		word_total;
	string_list_ty	punct;
	size_t		punct_total;
	size_t		total;
	char		*cp;
	int		j;		/* must be signed */
	size_t		word_max;
	static char	*buffer;
	static size_t	buffer_max;

	/*
	 * trivial sanity check
	 */
	assert(max > 0);
	total = s->str_length;
	if (total <= max || max <= 0)
		return str_copy(s);

	/*
	 * Break it all up into punctuation and words.
	 * The punctuation *preceeds* the word.
	 */
	string_list_constructor(&word);
	string_list_constructor(&punct);
	punct_total = 0;
	word_total = 0;
	cp = s->str_text;
	while (*cp)
	{
		char		*start;
		string_ty	*s1;

		start = cp;
		while (*cp && !isalnum((unsigned char)*cp))
			++cp;
		s1 = str_n_from_c(start, cp - start);
		string_list_append(&punct, s1);
		punct_total += s1->str_length;
		str_free(s1);

		start = cp;
		if (*cp && isdigit((unsigned char)*cp))
		{
			while (*cp && isdigit((unsigned char)*cp))
				++cp;
		}
		else
		{
			int was_upper = 1;
			while (*cp && isalpha((unsigned char)*cp))
			{
				int is_upper = isupper((unsigned char)*cp);
				if (!was_upper && is_upper)
					break;
				was_upper = is_upper;
				++cp;
			}
		}
		s1 = str_n_from_c(start, cp - start);
		string_list_append(&word, s1);
		word_total += s1->str_length;
		str_free(s1);
	}
	assert(punct.nstrings); /* can't be empty string */
	assert(punct.nstrings == word.nstrings);

	/*
	 * kill leading punctuation
	 */
	if (punct.string[0]->str_length)
	{
		total -= punct.string[0]->str_length;
		punct_total -= punct.string[0]->str_length;
		str_free(punct.string[0]);
		punct.string[0] = str_from_c("");
		if (total <= max)
			goto reassemble;
	}

	/*
	 * kill trailing punctuation
	 */
	if
	(
		punct.string[punct.nstrings - 1]->str_length
	&&
		!word.string[word.nstrings - 1]->str_length
	)
	{
		total -= punct.string[punct.nstrings - 1]->str_length;
		punct_total -= punct.string[punct.nstrings - 1]->str_length;
		str_free(punct.string[--punct.nstrings]);
		str_free(word.string[--word.nstrings]);
		if (total <= max)
			goto reassemble;
	}
	assert(punct.nstrings);
	assert(punct.nstrings == word.nstrings);

	/*
	 * shorten punctuation to one character each
	 */
	for (j = punct.nstrings - 1; j >= 0; --j)
	{
		string_ty	*s1;

		if (punct.string[j]->str_length < 2)
			continue;
		total -= punct.string[j]->str_length - 1;
		punct_total -= punct.string[j]->str_length - 1;
		s1 = str_n_from_c(punct.string[j]->str_text, (size_t)1);
		str_free(punct.string[j]);
		punct.string[j] = s1;
		if (total <= max)
			goto reassemble;
	}

	/*
	 * nuke all punctuation if we are very squeezed for space
	 * (except last dot, for file suffixes)
	 */
	if (punct_total * 5 > max)
	{
		for (j = punct.nstrings - 1; j >= 0; --j)
		{
			if (!punct.string[j]->str_length)
				continue;
			assert(punct.string[j]->str_length == 1);
			if
			(
				keep_last_dot
			&&
				j == punct.nstrings - 1
			&&
				punct.string[j]->str_text[0] == '.'
			)
				continue;
			--total;
			--punct_total;
			str_free(punct.string[j]);
			punct.string[j] = str_from_c("");
			if (total <= max)
				goto reassemble;
		}
	}

	/*
	 * shorten all the words
	 */
	word_max = 0;
	for (j = 0; j < word.nstrings; ++j)
		if (word.string[j]->str_length > word_max)
			word_max = word.string[j]->str_length;
	--word_max;
	while (word_max >= 1)
	{
		for (j = word.nstrings - 1; j >= 0; --j)
		{
			string_ty	*s1;

			if (word.string[j]->str_length <= word_max)
				continue;
			total -= (word.string[j]->str_length - word_max);
			s1 = str_n_from_c(word.string[j]->str_text, word_max);
			str_free(word.string[j]);
			word.string[j] = s1;
			if (total <= max)
				goto reassemble;
		}
		--word_max;
	}

reassemble:
	if (total > buffer_max)
	{
		buffer_max = total;
		buffer = mem_change_size(buffer, buffer_max);
	}
	cp = buffer;
	for (j = 0; j < punct.nstrings; ++j)
	{
		memcpy(cp, punct.string[j]->str_text, punct.string[j]->str_length);
		cp += punct.string[j]->str_length;
		memcpy(cp, word.string[j]->str_text, word.string[j]->str_length);
		cp += word.string[j]->str_length;
	}
	return str_n_from_c(buffer, total);
}


static string_ty *nuke_unprintable _((string_ty *));

static string_ty *
nuke_unprintable(s)
	string_ty	*s;
{
	static char	*buffer;
	static size_t	buffer_max;
	char		*ip;
	char		*op;
	int		c;

	if (s->str_length > buffer_max)
	{
		buffer_max = s->str_length;
		buffer = mem_change_size(buffer, buffer_max);
	}
	ip = s->str_text;
	op = buffer;
	while (*ip)
	{
		c = (unsigned char)*ip++;
		if (isspace(c) || !isprint(c))
			c = '_';
		*op++ = c;
	}
	return str_n_from_c(buffer, op - buffer);
}


string_ty *
abbreviate_dirname(s, max)
	string_ty	*s;
	int		max;
{
	string_ty	*s2;
	string_ty	*result;

	s2 = nuke_unprintable(s);
	result = abbreviate(s2, max, 0);
	str_free(s2);
	return result;
}


string_ty *
abbreviate_filename(s, max)
	string_ty	*s;
	int		max;
{
	string_ty	*s2;
	string_ty	*result;

	s2 = nuke_unprintable(s);
	result = abbreviate(s2, max, 1);
	str_free(s2);
	return result;
}


static int contains_moronic_ms_restrictions _((string_ty *));

static int
contains_moronic_ms_restrictions(fn)
	string_ty       *fn;
{
	static char     *moronic[] =
	{
		"aux",
		"com1",
		"com2",
		"com3",
		"com4",
		"con",
		"nul",
	};
	char		**cpp;
	string_ty	*fn2;
	static string_list_ty wl;
	int		result;
	
	if (wl.nstrings == 0)
	{
		for (cpp = moronic; cpp < ENDOF(moronic); ++cpp)
		{
			fn2 = str_from_c(*cpp);
			string_list_append(&wl, fn2);
			str_free(fn2);
		}
	}

	fn2 = str_downcase(fn);
	result = string_list_member(&wl, fn2);
	str_free(fn2);
	return result;
}

string_ty *
abbreviate_8dos3(s)
	string_ty	*s;
{
	string_ty	*s1;
	char		*cp;
	string_ty	*result;

	s1 = nuke_unprintable(s);
	cp = strrchr(s1->str_text, '.');
	if (!cp)
		result = abbreviate(s1, 8, 0);
	else
	{
		string_ty	*s2;
		string_ty	*s2a;
		string_ty	*s3;
		string_ty	*s3a;

		s2 = str_n_from_c(s1->str_text, cp - s1->str_text);
		++cp;
		s2a = abbreviate(s2, 8, 0);
		if
		(
			s2a->str_length == 0
		||
			!isalpha(s2a->str_text[0])
		||
			contains_moronic_ms_restrictions(s2a)
		)
		{
			string_ty	*s4;

			str_free(s2a);
			s4 = str_format("a%S", s2);
			s2a = abbreviate(s4, 8, 0);
			str_free(s4);
		}
		str_free(s2);
		s3 = str_n_from_c(cp, s1->str_text + s1->str_length - cp);
		s3a = abbreviate(s3, 3, 0);
		str_free(s3);
		if (s3a->str_length)
			result = str_format("%S.%S", s2a, s3a);
		else
			result = str_copy(s2a);
		str_free(s2a);
		str_free(s3a);
	}
	str_free(s1);
	return result;
}
