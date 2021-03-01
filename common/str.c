/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * MANIFEST: string manipulation functions
 *
 * A literal pool is maintained.  Each string has a reference count.  The
 * string stays in the literal pool for as long as it has a positive
 * reference count.  To determine if a string is already in the literal pool,
 * linear dynamic hashing is used to guarantee an O(1) search.  Making all equal
 * strings the same item in the literal pool means that string equality is
 * a pointer test, and thus very fast.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <error.h>
#include <mem.h>
#include <s-v-arg.h>
#include <str.h>


/*
 * maximum conversion width for numbers
 */
#define MAX_WIDTH 509

static	string_ty	**hash_table;
static	str_hash_ty	hash_modulus;
static	str_hash_ty	hash_cutover;
static	str_hash_ty	hash_cutover_mask;
static	str_hash_ty	hash_cutover_split_mask;
static	str_hash_ty	hash_split;
static	str_hash_ty	hash_load;
static	int		changed;

#define MAX_HASH_LEN 20


/*
 * NAME
 *	hash_generate - hash string to number
 *
 * SYNOPSIS
 *	str_hash_ty hash_generate(char *s, size_t n);
 *
 * DESCRIPTION
 *	The hash_generate function is used to make a number from a string.
 *
 * RETURNS
 *	str_hash_ty - the magic number
 *
 * CAVEAT
 *	Only the last MAX_HASH_LEN characters are used.
 *	It is important that str_hash_ty be unsigned (int or long).
 */

static str_hash_ty hash_generate _((char *, size_t));

static str_hash_ty
hash_generate(s, n)
	char		*s;
	size_t		n;
{
	str_hash_ty	retval;

	if (n > MAX_HASH_LEN)
	{
		s += n - MAX_HASH_LEN;
		n = MAX_HASH_LEN;
	}

	retval = 0;
	while (n > 0)
	{
		retval = (retval + (retval << 1)) ^ *s++;
		--n;
	}
	return retval;
}


/*
 * NAME
 *	str_initialize - start up string table
 *
 * SYNOPSIS
 *	void str_initialize(void);
 *
 * DESCRIPTION
 *	The str_initialize function is used to create the hash table and
 *	initialize it to empty.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	This function must be called before any other defined in this file.
 */

void
str_initialize()
{
	str_hash_ty	j;

	hash_modulus = 1<<8; /* MUST be a power of 2 */
	hash_cutover = hash_modulus;
	hash_split = hash_modulus - hash_cutover;
	hash_cutover_mask = hash_cutover - 1;
	hash_cutover_split_mask = (hash_cutover * 2) - 1;
	hash_load = 0;
	hash_table = (string_ty **)mem_alloc(hash_modulus * sizeof(string_ty *));
	for (j = 0; j < hash_modulus; ++j)
		hash_table[j] = 0;
}


/*
 * NAME
 *	split - reduce table loading
 *
 * SYNOPSIS
 *	void split(void);
 *
 * DESCRIPTION
 *	The split function is used to reduce the load factor on the hash table.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	A load factor of about 80% is suggested.
 */

static void split _((void));

static void
split()
{
	string_ty	*p;
	string_ty	*p2;
	str_hash_ty	index;

	/*
	 * get the list to be split across buckets
	 */
	p = hash_table[hash_split];
	hash_table[hash_split] = 0;

	/*
	 * increase the modulus by one
	 */
	hash_modulus++;
	mem_change_size((char **)&hash_table, hash_modulus * sizeof(string_ty*));
	hash_table[hash_modulus - 1] = 0;
	hash_split = hash_modulus - hash_cutover;
	if (hash_split >= hash_cutover)
	{
		hash_cutover = hash_modulus;
		hash_split = 0;
		hash_cutover_mask = hash_cutover - 1;
		hash_cutover_split_mask = (hash_cutover * 2) - 1;
	}

	/*
	 * now redistribute the list elements
	 */
	while (p)
	{
		p2 = p;
		p = p->str_next;

		index = p2->str_hash & hash_cutover_mask;
		if (index < hash_split)
			index = p2->str_hash & hash_cutover_split_mask;
		p2->str_next = hash_table[index];
		hash_table[index] = p2;
	}
}


/*
 * NAME
 *	str_from_c - make string from C string
 *
 * SYNOPSIS
 *	string_ty *str_from_c(char*);
 *
 * DESCRIPTION
 *	The str_from_c function is used to make a string from a null terminated
 *	C string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_from_c(s)
	char		*s;
{
	return str_n_from_c(s, strlen(s));
}


/*
 * NAME
 *	str_n_from_c - make string
 *
 * SYNOPSIS
 *	string_ty *str_n_from_c(char *s, size_t n);
 *
 * DESCRIPTION
 *	The str_n_from_c function is used to make a string from an array of
 *	characters.  No null terminator is assumed.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_n_from_c(s, length)
	char		*s;
	size_t		length;
{
	str_hash_ty	hash;
	str_hash_ty	index;
	string_ty	*p;

	hash = hash_generate(s, length);

	index = hash & hash_cutover_mask;
	if (index < hash_split)
		index = hash & hash_cutover_split_mask;

	for (p = hash_table[index]; p; p = p->str_next)
	{
		if
		(
			p->str_hash == hash
		&&
			p->str_length == length
		&&
			!memcmp(p->str_text, s, length)
		)
		{
			p->str_references++;
			return p;
		}
	}

	p = (string_ty *)mem_alloc(sizeof(string_ty) + length);
	p->str_hash = hash;
	p->str_length = length;
	p->str_references = 1;
	p->str_next = hash_table[index];
	hash_table[index] = p;
	memcpy(p->str_text, s, length);
	p->str_text[length] = 0;

	hash_load++;
	while (hash_load * 10 > hash_modulus * 8)
		split();
	++changed;
	return p;
}


/*
 * NAME
 *	str_copy - make a copy of a string
 *
 * SYNOPSIS
 *	string_ty *str_copy(string_ty *s);
 *
 * DESCRIPTION
 *	The str_copy function is used to make a copy of a string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_copy(s)
	string_ty	*s;
{
	s->str_references++;
	return s;
}


/*
 * NAME
 *	str_free - release a string
 *
 * SYNOPSIS
 *	void str_free(string_ty *s);
 *
 * DESCRIPTION
 *	The str_free function is used to indicate that a string hash been
 *	finished with.
 *
 * RETURNS
 *	void
 *
 * CAVEAT
 *	This is the only way to release strings DO NOT use the free function.
 */

void
str_free(s)
	string_ty	*s;
{
	str_hash_ty	index;
	string_ty	**spp;

	if (!s)
		return;
	if (s->str_references > 1)
	{
		s->str_references--;
		return;
	}
	++changed;

	/*
	 * find the hash bucket it was in,
	 * and remove it
	 */
	index = s->str_hash & hash_cutover_mask;
	if (index < hash_split)
		index = s->str_hash & hash_cutover_split_mask;
	for (spp = &hash_table[index]; *spp; spp = &(*spp)->str_next)
	{
		if (*spp == s)
		{
			*spp = s->str_next;
			free(s);
			--hash_load;
			return;
		}
	}

	/*
	 * should never reach here!
	 */
	fatal("attempted to free non-existent string (bug)");
}


/*
 * NAME
 *	str_catenate - join two strings
 *
 * SYNOPSIS
 *	string_ty *str_catenate(string_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The str_catenate function is used to concatenate two strings to form a
 *	new string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_catenate(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	static char	*tmp;
	static size_t	tmplen;
	string_ty	*s;
	size_t		length;

	length = s1->str_length + s2->str_length;
	if (!tmp)
	{
		tmplen = length;
		if (tmplen < 16)
			tmplen = 16;
		tmp = mem_alloc(tmplen);
	}
	else
	{
		if (tmplen < length)
		{
			tmplen = length;
			mem_change_size(&tmp, tmplen);
		}
	}
	memcpy(tmp, s1->str_text, s1->str_length);
	memcpy(tmp + s1->str_length, s2->str_text, s2->str_length);
	s = str_n_from_c(tmp, length);
	return s;
}


/*
 * NAME
 *	str_cat_three - join three strings
 *
 * SYNOPSIS
 *	string_ty *str_cat_three(string_ty *, string_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The str_cat_three function is used to concatenate three strings to form
 *	a new string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_cat_three(s1, s2, s3)
	string_ty	*s1;
	string_ty	*s2;
	string_ty	*s3;
{
	static char	*tmp;
	static size_t	tmplen;
	string_ty	*s;
	size_t		length;

	length = s1->str_length + s2->str_length + s3->str_length;
	if (!tmp)
	{
		tmplen = length;
		if (tmplen < 16)
			tmplen = 16;
		tmp = mem_alloc(tmplen);
	}
	else
	{
		if (tmplen < length)
		{
			tmplen = length;
			mem_change_size(&tmp, tmplen);
		}
	}
	memcpy(tmp, s1->str_text, s1->str_length);
	memcpy(tmp + s1->str_length, s2->str_text, s2->str_length);
	memcpy
	(
		tmp + s1->str_length + s2->str_length,
		s3->str_text,
		s3->str_length
	);
	s = str_n_from_c(tmp, length);
	return s;
}


/*
 * NAME
 *	str_equal - test equality of strings
 *
 * SYNOPSIS
 *	int str_equal(string_ty *, string_ty *);
 *
 * DESCRIPTION
 *	The str_equal function is used to test if two strings are equal.
 *
 * RETURNS
 *	int; zero if the strings are not equal, nonzero if the strings are
 *	equal.
 *
 * CAVEAT
 *	This function is implemented as a macro in strings.h
 */

#ifndef str_equal

int
str_equal(s1, s2)
	string_ty	*s1;
	string_ty	*s2;
{
	return (s1 == s2);
}

#endif


/*
 * NAME
 *	str_upcase - upcase a string
 *
 * SYNOPSIS
 *	string_ty *str_upcase(string_ty *);
 *
 * DESCRIPTION
 *	The str_upcase function is used to form a string which is an upper case
 *	form of the supplied string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_upcase(s)
	string_ty	*s;
{
	static char	*tmp;
	static size_t	tmplen;
	string_ty	*retval;
	char		*cp1;
	char		*cp2;

	if (!tmp)
	{
		tmplen = s->str_length;
		if (tmplen < 16)
			tmplen = 16;
		tmp = mem_alloc(tmplen);
	}
	else
	{
		if (tmplen < s->str_length)
		{
			tmplen = s->str_length;
			mem_change_size(&tmp, tmplen);
		}
	}
	for (cp1 = s->str_text, cp2 = tmp; *cp1; ++cp1, ++cp2)
	{
		int c;

		c = *cp1;
		if (c >= 'a' && c <= 'z')
			c += 'A' - 'a';
		*cp2 = c;
	}
	retval = str_n_from_c(tmp, s->str_length);
	return retval;
}


/*
 * NAME
 *	str_downcase - lowercase string
 *
 * SYNOPSIS
 *	string_ty *str_downcase(string_ty *);
 *
 * DESCRIPTION
 *	The str_downcase function is used to form a string which is a lowercase
 *	form of the supplied string.
 *
 * RETURNS
 *	string_ty* - a pointer to a string in dynamic memory.  Use str_free when
 *	finished with.
 *
 * CAVEAT
 *	The contents of the structure pointed to MUST NOT be altered.
 */

string_ty *
str_downcase(s)
	string_ty	*s;
{
	static char	*tmp;
	static size_t	tmplen;
	string_ty	*retval;
	char		*cp1;
	char		*cp2;

	if (!tmp)
	{
		tmplen = s->str_length;
		if (tmplen < 16)
			tmplen = 16;
		tmp = mem_alloc(tmplen);
	}
	else
	{
		if (tmplen < s->str_length)
		{
			tmplen = s->str_length;
			mem_change_size(&tmp, tmplen);
		}
	}
	for (cp1 = s->str_text, cp2 = tmp; *cp1; ++cp1, ++cp2)
	{
		int c;

		c = *cp1;
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		*cp2 = c;
	}
	retval = str_n_from_c(tmp, s->str_length);
	return retval;
}


/*
 * NAME
 *	str_bool - get boolean value
 *
 * SYNOPSIS
 *	int str_bool(string_ty *s);
 *
 * DESCRIPTION
 *	The str_bool function is used to determine the boolean value of the
 *	given string.  A "false" result is if the string is empty or
 *	0 or blank, and "true" otherwise.
 *
 * RETURNS
 *	int: zero to indicate a "false" result, nonzero to indicate a "true"
 *	result.
 */

int
str_bool(s)
	string_ty	*s;
{
	char		*cp;

	cp = s->str_text;
	while (*cp)
	{
		if (!isspace(*cp) && *cp != '0')
			return 1;
		++cp;
	}
	return 0;
}


/*
 * NAME
 *	str_field - extract a field from a string
 *
 * SYNOPSIS
 *	string_ty *str_field(string_ty *, char separator, int field_number);
 *
 * DESCRIPTION
 *	The str_field functipon is used to erxtract a field from a string.
 *	Fields of the string are separated by ``separator'' characters.
 *	Fields are numbered from 0.
 *
 * RETURNS
 *	Asking for a field off the end of the string will result in a null
 *	pointer return.  The null string is considered to have one empty field.
 */

string_ty *
str_field(s, sep, fldnum)
	string_ty	*s;
	int		sep;
	int		fldnum;
{
	char		*cp;
	char		*ep;

	cp = s->str_text;
	while (fldnum > 0)
	{
		ep = strchr(cp, sep);
		if (!ep)
			return 0;
		cp = ep + 1;
		--fldnum;
	}
	ep = strchr(cp, sep);
	if (ep)
		return str_n_from_c(cp, ep - cp);
	return str_from_c(cp);
}


void
slow_to_fast(in, out, length)
	char		**in;
	string_ty	**out;
	size_t		length;
{
	size_t		j;

	if (out[0])
		return;
	for (j = 0; j < length; ++j)
		out[j] = str_from_c(in[j]);
}


string_ty *
str_format(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	string_ty	*result;

	sva_init(ap, fmt);
	result = str_vformat(fmt, ap);
	va_end(ap);
	return result;
}


static void build_fake _((char *fake, int flag, int width, int precision,
	int qualifier, int specifier));

static void
build_fake(fake, flag, width, precision, qualifier, specifier)
	char		*fake;
	int		flag;
	int		width;
	int		precision;
	int		qualifier;
	int		specifier;
{
	char		*fp;

	fp = fake;
	*fp++ = '%';
	if (flag)
		*fp++ = flag;
	if (width > 0)
	{
		sprintf(fp, "%d", width);
		fp += strlen(fp);
	}
	*fp++ = '.';
	sprintf(fp, "%d", precision);
	fp += strlen(fp);
	if (qualifier)
		*fp++ = qualifier;
	*fp++ = specifier;
	*fp = 0;
}


string_ty *
str_vformat(fmt, ap)
	char		*fmt;
	va_list		ap;
{
	size_t		length;
	static size_t	tmplen;
	static char	*tmp;
	int		width;
	int		width_set;
	int		prec;
	int		prec_set;
	int		c;
	char		*s;
	int		qualifier;
	int		flag;
	char		fake[100];

	/*
	 * Build the result string in a temporary buffer.
	 * Grow the temporary buffer as necessary.
	 *
	 * It is important to only make one pass across the variable argument
	 * list.  Behaviour is undefined for more than one pass.
	 */
	if (!tmplen)
	{
		tmplen = 500;
		tmp = mem_alloc(tmplen);
	}
	length = 0;
	s = fmt;
	while (*s)
	{
		c = *s++;
		if (c != '%')
		{
			normal:
			if (length >= tmplen)
			{
				tmplen += 100;
				mem_change_size(&tmp, tmplen);
			}
			tmp[length++] = c;
			continue;
		}
		c = *s++;

		/*
		 * get optional flag
		 */
		switch (c)
		{
		case '+':
		case '-':
		case '#':
		case '0':
		case ' ':
			flag = c;
			c = *s++;
			break;

		default:
			flag = 0;
			break;
		}

		/*
		 * get optional width
		 */
		width = 0;
		width_set = 0;
		switch (c)
		{
		case '*':
			width = va_arg(ap, int);
			if (width < 0)
			{
				flag = '-';
				width = -width;
			}
			c = *s++;
			width_set = 1;
			break;
		
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (;;)
			{
				width = width * 10 + c - '0';
				c = *s++;
				switch (c)
				{
				default:
					break;

				case '0': case '1': case '2': case '3':
				case '4': case '5': case '6': case '7':
				case '8': case '9':
					continue;
				}
				break;
			}
			width_set = 1;
			break;

		default:
			break;
		}

		/*
		 * get optional precision
		 */
		prec = 0;
		prec_set = 0;
		if (c == '.')
		{
			c = *s++;
			switch (c)
			{
			default:
				prec_set = 1;
				break;

			case '*':
				c = *s++;
				prec = va_arg(ap, int);
				if (prec < 0)
				{
					prec = 0;
					break;
				}
				prec_set = 1;
				break;

			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				for (;;)
				{
					prec = prec * 10 + c - '0';
					c = *s++;
					switch (c)
					{
					default:
						break;

					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						continue;
					}
					break;
				}
				prec_set = 1;
				break;
			}
		}

		/*
		 * get the optional qualifier
		 */
		switch (c)
		{
		default:
			qualifier = 0;
			break;

		case 'l':
		case 'h':
		case 'L':
			qualifier = c;
			c = *s++;
			break;
		}

		/*
		 * get conversion specifier
		 */
		switch (c)
		{
		default:
			fatal
			(
			      "in format string \"%s\", illegal specifier '%c'",
				fmt,
				c
			);

		case '%':
			goto normal;

		case 'c':
			{
				int	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				a = (unsigned char)va_arg(ap, int);
				if (!prec_set)
					prec = 1;
				if (width > MAX_WIDTH)
					width = MAX_WIDTH;
				if (prec > MAX_WIDTH)
					prec = MAX_WIDTH;
				build_fake(fake, flag, width, prec, 0, c);
				sprintf(num, fake, a);
				len = strlen(num);
				if (length + len > tmplen)
				{
					tmplen += len;
					mem_change_size(&tmp, tmplen);
				}
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 'd':
		case 'i':
			{
				long	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				switch (qualifier)
				{
				case 'l':
					a = va_arg(ap, long);
					break;

				case 'h':
					a = (short)va_arg(ap, int);
					break;

				default:
					a = va_arg(ap, int);
					break;
				}
				if (!prec_set)
					prec = 1;
				if (width > MAX_WIDTH)
					width = MAX_WIDTH;
				if (prec > MAX_WIDTH)
					prec = MAX_WIDTH;
				build_fake(fake, flag, width, prec, 'l', c);
				sprintf(num, fake, a);
				len = strlen(num);
				if (length + len > tmplen)
				{
					tmplen += len;
					mem_change_size(&tmp, tmplen);
				}
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 'e':
		case 'f':
		case 'g':
		case 'E':
		case 'F':
		case 'G':
			{
				double	a;
				char	num[MAX_WIDTH + 1];
				size_t	len;

				/*
				 * Ignore "long double" for now,
				 * traditional implementations no grok.
				 */
				a = va_arg(ap, double);
				if (!prec_set)
					prec = 6;
				if (width > MAX_WIDTH)
					width = MAX_WIDTH;
				if (prec > MAX_WIDTH)
					prec = MAX_WIDTH;
				build_fake(fake, flag, width, prec, 0, c);
				sprintf(num, fake, a);
				len = strlen(num);
				if (length + len > tmplen)
				{
					tmplen += len;
					mem_change_size(&tmp, tmplen);
				}
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 'n':
			switch (qualifier)
			{
			case 'l':
				{
					long	*a;

					a = va_arg(ap, long *);
					*a = length;
				}
				break;

			case 'h':
				{
					short	*a;

					a = va_arg(ap, short *);
					*a = length;
				}
				break;

			default:
				{
					int	*a;

					a = va_arg(ap, int *);
					*a = length;
				}
				break;
			}
			break;

		case 'u':
		case 'o':
		case 'x':
		case 'X':
			{
				unsigned long	a;
				char		num[MAX_WIDTH + 1];
				size_t		len;

				switch (qualifier)
				{
				case 'l':
					a = va_arg(ap, unsigned long);
					break;

				case 'h':
					a = (unsigned short)va_arg(ap, unsigned int);
					break;

				default:
					a = va_arg(ap, unsigned int);
					break;
				}
				if (!prec_set)
					prec = 1;
				if (prec > MAX_WIDTH)
					prec = MAX_WIDTH;
				if (width > MAX_WIDTH)
					width = MAX_WIDTH;
				build_fake(fake, flag, width, prec, 'l', c);
				sprintf(num, fake, a);
				len = strlen(num);
				if (length + len > tmplen)
				{
					tmplen += len;
					mem_change_size(&tmp, tmplen);
				}
				memcpy(tmp + length, num, len);
				length += len;
			}
			break;

		case 's':
			{
				char	*a;
				size_t	len;

				a = va_arg(ap, char *);
				if (prec_set)
				{
					char	*ep;

					ep = memchr(a, 0, prec);
					if (ep)
						len = ep - a;
					else
						len = prec;
				}
				else
					len = strlen(a);
				if (!prec_set || len < prec)
					prec = len;
				if (!width_set || width < prec)
					width = prec;
				len = width;
				if (length + len > tmplen)
				{
					tmplen += len;
					mem_change_size(&tmp, tmplen);
				}
				if (flag != '-')
				{
					while (width > prec)
					{
						tmp[length++] = ' ';
						width--;
					}
				}
				memcpy(tmp + length, a, prec);
				length += prec;
				width -= prec;
				if (flag == '-')
				{
					while (width > 0)
					{
						tmp[length++] = ' ';
						width--;
					}
				}
			}
			break;

		case 'S':
			{
				string_ty	*a;
				size_t		len;

				a = va_arg(ap, string_ty *);
				len = a->str_length;
				if (!prec_set)
					prec = len;
				if (len < prec)
					prec = len;
				if (!width_set)
					width = prec;
				if (width < prec)
					width = prec;
				len = width;
				if (length + len > tmplen)
				{
					tmplen += len;
					mem_change_size(&tmp, tmplen);
				}
				if (flag != '-')
				{
					while (width > prec)
					{
						tmp[length++] = ' ';
						width--;
					}
				}
				memcpy(tmp + length, a->str_text, prec);
				length += prec;
				width -= prec;
				if (flag == '-')
				{
					while (width > 0)
					{
						tmp[length++] = ' ';
						width--;
					}
				}
			}
			break;
		}
	}

	/*
	 * make the string
	 */
	assert(length <= tmplen);
	return str_n_from_c(tmp, length);
}
