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
 * MANIFEST: functions to manipulate headers
 */

#include <ac/ctype.h>

#include <header.h>
#include <input/822wrap.h>
#include <input/crlf.h>
#include <mem.h>
#include <str.h>
#include <stracc.h>
#include <symtab.h>


static void reap _((void *));

static void
reap(p)
	void		*p;
{
	string_ty	*s;

	s = p;
	str_free(s);
}


static int has_a_header _((input_ty *));

static int
has_a_header(ifp)
	input_ty	*ifp;
{
	stracc_t	buffer;
	int		result;
	int		state;
	int		c;

	result = 0;
	stracc_constructor(&buffer);
	stracc_open(&buffer);
	state = 0;
	while (state < 3 && buffer.length < 80)
	{
		c = input_getc(ifp);
		if (c >= 0)
			stracc_char(&buffer, c);
		switch (c)
		{
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z': 
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': 
			if (state == 0 || state == 1)
				state = 1;
			else
				state = 3;
			break;

		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '-':
			if (state != 1)
				state = 3;
			break;

		case ':': 
			if (state == 1)
				state = 2;
			else
				state = 3;
			break;

		case ' ':
			if (state == 2)
				result = 1;
			state = 3;
			break;
		
		default:
			state = 3;
			break;
		}
	}
	while (buffer.length > 0)
	{
		c = (unsigned char)buffer.buffer[--buffer.length];
		input_ungetc(ifp, c);
	}
	stracc_destructor(&buffer);
	return result;
}


static int is_binary _((input_ty *));

static int
is_binary(ifp)
	input_ty	*ifp;
{
	int		result;
	int		c;
	stracc_t	buffer;

	/*
	 * We are looking for NULs, because gzipped files have
	 * NULs in the header.	This function is only used to fake a
	 * Content-Transfer-Encoding line in there case where there is
	 * no header to tell us.
	 */
	result = 0;
	stracc_constructor(&buffer);
	stracc_open(&buffer);
	while (buffer.length < 800)
	{
		c = input_getc(ifp);
		if (c < 0)
			break;
		stracc_char(&buffer, c);
		if (c == 0)
		{
			result = 1;
			break;
		}
	}
	while (buffer.length > 0)
	{
		c = (unsigned char)buffer.buffer[--buffer.length];
		input_ungetc(ifp, c);
	}
	stracc_destructor(&buffer);
	return result;
}


header_ty *
header_read(ifp)
	input_ty	*ifp;
{
	header_ty	*hp;
	stracc_t	buffer;

	/*
	 * Allocate a symbol table to hold everything.
	 */
	hp = mem_alloc(sizeof(header_ty));
	hp->stp = symtab_alloc(1);
	hp->stp->reap = reap;

	/*
	 * If there is no 822 header, return an empty symbol table.
	 */
	if (!has_a_header(ifp))
	{
		if (!is_binary(ifp))
		{
			string_ty	*name;
			string_ty	*value;

			name = str_from_c("content-transfer-encoding");
			value = str_from_c("base64");
			symtab_assign(hp->stp, name, value);
			str_free(name);
		}
		return hp;
	}

	/*
	 * Do the end-of-line wrapping transparently.
	 */
	ifp = input_crlf(ifp, 0);
	ifp = input_822wrap(ifp, 1);

	/*
	 * Read header lines until we find a blank line.
	 */
	stracc_constructor(&buffer);
	for (;;)
	{
		int		c;
		string_ty	*name;
		string_ty	*value;

		/*
		 * EOF or a blank line stops us.
		 */
		c = input_getc(ifp);
		if (c < 0)
			break;
		if (c == '\n')
			break;

		/*
		 * Read the name.
		 * Complain if we don't like the characters.
		 */
		stracc_open(&buffer);
		for (;;)
		{
			if (c == ':')
				break;
			if (c != '-' && !isalnum(c))
				input_format_error(ifp);
			if (isupper(c))
				c = tolower(c);
			stracc_char(&buffer, c);
			c = input_getc(ifp);
		}
		name = stracc_close(&buffer);
		if (name->str_length < 1)
			input_format_error(ifp);

		/*
		 * Skip the white space after the colon.
		 */
		for (;;)
		{
			c = input_getc(ifp);
			if (c != ' ' && c != '\t')
				break;
		}

		/*
		 * Read the value.
		 * We aren't so picky this time.
		 */
		stracc_open(&buffer);
		for (;;)
		{
			if (c < 0 || c == '\n')
				break;
			stracc_char(&buffer, c);
			c = input_getc(ifp);
		}
		value = stracc_close(&buffer);

		/*
		 * assign the value
		 */
		symtab_assign(hp->stp, name, value);
		str_free(name);
	}
	stracc_destructor(&buffer);
	input_delete(ifp);
	return hp;
}


void
header_delete(hp)
	header_ty	*hp;
{
	symtab_free(hp->stp);
	mem_free(hp);
}


string_ty *
header_query(hp, name)
	header_ty	*hp;
	const char	*name;
{
	string_ty	*Name;
	string_ty	*Name2;
	string_ty	*result;

	Name = str_from_c(name);
	Name2 = str_downcase(Name);
	str_free(Name);
	result = symtab_query(hp->stp, Name2);
	str_free(Name2);
	return result;
}
