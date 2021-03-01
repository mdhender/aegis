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
 * MANIFEST: functions to manipulate base64s
 */

#include <input/base64.h>
#include <input/private.h>


typedef struct input_base64_ty input_base64_ty;
struct input_base64_ty
{
	input_ty	inherited;
	input_ty	*deeper;
	long		pos;
	int		residual_bits;
	int		residual_value;
	int		eof;
};


static void destruct _((input_ty *));

static void
destruct(fp)
	input_ty	*fp;
{
	input_base64_ty	*this;

	this = (input_base64_ty *)fp;
	input_delete(this->deeper);
	this->deeper = 0; /* paranoia */
}


static int get _((input_ty *));

static int
get(fp)
	input_ty	*fp;
{
	input_base64_ty	*this;
	int		c;

	this = (input_base64_ty *)fp;
	if (this->eof)
		return -1;
	while (this->residual_bits < 8)
	{
		c = input_getc(this->deeper);
		switch (c)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
			continue;

		case '=':
			this->eof = 1;
			return -1;

		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
		case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
		case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
		case 'Y': case 'Z': 
			/*
			 * This next statement is not portable to
			 * non-ascii character sets, because A-Z are not
			 * guaranteed to be continuous.
			 */
			c = c - 'A';
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		case 'y': case 'z': 
			/*
			 * This next statement is not portable to
			 * non-ascii character sets, because a-z are not
			 * guaranteed to be continuous.
			 */
			c = c - 'a' + 26;
			break;

		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
			c = c - '0' + 52;
			break;

		case '+': 
			c = 62;
			break;

		case '/': 
			c = 63;
			break;

		default:
			if (c < 0)
			{
				if (this->residual_bits != 0)
					input_format_error(fp);
				this->eof = 1;
				return -1;
			}
			input_format_error(fp);
			/* NOTREACHED */
		}
		this->residual_value = (this->residual_value << 6) + c;
		this->residual_bits += 6;
	}
	this->pos++;
	this->residual_bits -= 8;
	c = (this->residual_value >> this->residual_bits) & 0xFF;
	return c;
}


static long itell _((input_ty *));

static long
itell(deeper)
	input_ty	*deeper;
{
	input_base64_ty	*this;

	this = (input_base64_ty *)deeper;
	return this->pos;
}


static const char *name _((input_ty *));

static const char *
name(fp)
	input_ty	*fp;
{
	input_base64_ty	*this;

	this = (input_base64_ty *)fp;
	return input_name(this->deeper);
}


static long length _((input_ty *));

static long
length(fp)
	input_ty	*fp;
{
	return -1;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_base64_ty),
	destruct,
	input_generic_read,
	get,
	itell,
	name,
	length,
};


input_ty *
input_base64(deeper)
	input_ty	*deeper;
{
	input_ty	*result;
	input_base64_ty	*this;

	result = input_new(&vtbl);
	this = (input_base64_ty *)result;
	this->deeper = deeper;
	this->pos = 0;
	this->residual_bits = 0;
	this->residual_value = 0;
	this->eof = 0;
	return result;
}
