/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions for reading input and filtering out CR LF sequences
 */

#include <error.h>
#include <input/crlf.h>
#include <input/private.h>
#include <str.h>
#include <sub.h>
#include <trace.h>


typedef struct input_crlf_ty input_crlf_ty;
struct input_crlf_ty
{
	input_ty	inherited;
	input_ty	*deeper;
	long		pos;
	int		delete_on_close;
	long		line_number;
	int		prev_was_newline;
	string_ty	*name_cache;
	int		escaped_newline;
};


static void
input_crlf_destructor(input_ty *p)
{
	input_crlf_ty	*this_thing;

	trace(("input_crlf_destructor()\n{\n"));
	this_thing = (input_crlf_ty *)p;
	input_pushback_transfer(this_thing->deeper, p);
	if (this_thing->delete_on_close)
		input_delete(this_thing->deeper);
	this_thing->deeper = 0; /* paranoia */
	if (this_thing->name_cache)
	{
		str_free(this_thing->name_cache);
		this_thing->name_cache = 0;
	}
	trace(("}\n"));
}


static long
input_crlf_read(input_ty *ip, void *data, size_t len)
{
	input_crlf_ty	*this_thing;
	int		c;
	unsigned char	*cp;
	unsigned char	*end;
	size_t		nbytes;
	sub_context_ty	*scp;

	trace(("input_crlf_read()\n{\n"));
	this_thing = (input_crlf_ty *)ip;
	if (this_thing->prev_was_newline)
	{
		this_thing->line_number++;
		this_thing->prev_was_newline = 0;

		/*
		 * The name cache includes the line number, and the line
		 * number just changed, so nuke it.
		 */
		if (this_thing->name_cache)
		{
			str_free(this_thing->name_cache);
			this_thing->name_cache = 0;
		}
	}

	cp = (unsigned char *)data;
	end = cp + len;
	while (cp < end)
	{
		c = input_getc(this_thing->deeper);
		switch (c)
		{
		case '\r':
			c = input_getc(this_thing->deeper);
			if (c == '\n')
				goto newline;
			if (c >= 0)
				input_ungetc(this_thing->deeper, c);
#ifdef __mac_os_x__
			goto newline;
#else
			*cp++ = '\r';
			continue;
#endif

		case '\\':
			if (this_thing->escaped_newline)
			{
				c = input_getc(this_thing->deeper);
				if (c == '\n')
				{
					/*
					 * Don't put the newline in the
					 * buffer, but DO stop here,
					 * so that the line numbers
					 * are right.
					 */
					this_thing->prev_was_newline = 1;
					break;
				}
				if (c >= 0)
					input_ungetc(this_thing->deeper, c);
			}
			*cp++ = '\\';
			continue;

		case -1:
			break;

		case 0:
			/*
			 * For plain ASCII text, the conditions reads
			 *
			 *	if (!isprint((unsigned char)c) &&
			 *		!isspace((unsigned char)c))
			 *
			 * However, for international text, just about
			 * anything is acceptable.  But not NUL.
			 */
			scp = sub_context_new();
			sub_var_set_format(scp, "Name", "\\%o", c);
			fatal_intl(scp, i18n("illegal '$name' character"));
			sub_context_delete(scp);
			continue;

		default:
			/*
			 * The default should be enough, but these are
			 * to force the use of a lookup table instead of
			 * an if-then-else chain in the code generated for
			 * the switch.
			 */
		case '!': case '"': case '#': case '$': case '%': case '&':
		case '\'': case '(': case ')': case '*': case '+': case ',':
		case '-': case '.': case '/': case '0': case '1': case '2':
		case '3': case '4': case '5': case '6': case '7': case '8':
		case '9': case ':': case ';': case '<': case '=': case '>':
		case '?': case '@': case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
		case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V':
		case 'W': case 'X': case 'Y': case 'Z': case '[': case ']':
		case '^': case '_': case '`': case 'a': case 'b': case 'c':
		case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
		case 'j': case 'k': case 'l': case 'm': case 'n': case 'o':
		case 'p': case 'q': case 'r': case 's': case 't': case 'u':
		case 'v': case 'w': case 'x': case 'y': case 'z': case '{':
		case '|': case '}': case '~':
			*cp++ = c;
			continue;

		case '\n':
			/*
			 * We are line buffered.  This is the best way to
			 * allow us to report line numbers accurately.
			 * It results is some inefficiencies, but it also
			 * lets us get rid of a whole layer of function
			 * calls in the lexer.
			 */
			newline:
			*cp++ = '\n';
			this_thing->prev_was_newline = 1;
			break;
		}
		break;
	}

	/*
	 * Figure what happened.
	 */
	nbytes = (cp - (unsigned char *)data);
	this_thing->pos += nbytes;
	trace(("return %ld;\n", (long)nbytes));
	trace(("}\n"));
	return nbytes;
}


static long
input_crlf_ftell(input_ty *fp)
{
	input_crlf_ty	*this_thing;

	this_thing = (input_crlf_ty *)fp;
	trace(("input_crlf_ftell => %ld\n", this_thing->pos));
	return this_thing->pos;
}


static string_ty *
input_crlf_name(input_ty *p)
{
	input_crlf_ty	*this_thing;

	trace(("input_crlf_name\n"));
	this_thing = (input_crlf_ty *)p;
	if (!this_thing->line_number)
		return input_name(this_thing->deeper);
	if (!this_thing->name_cache)
	{
		this_thing->name_cache =
			str_format
			(
				"%S: %ld",
				input_name(this_thing->deeper),
				this_thing->line_number
			);
	}
	return this_thing->name_cache;
}


static long
input_crlf_length(input_ty *p)
{
	trace(("input_crlf_length => -1\n"));
	return -1;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_crlf_ty),
	input_crlf_destructor,
	input_crlf_read,
	input_crlf_ftell,
	input_crlf_name,
	input_crlf_length,
};


input_ty *
input_crlf(input_ty *fp, int delete_on_close)
{
	input_ty	*result;
	input_crlf_ty	*this_thing;

	trace(("input_crlf(fp = %08lX)\n{\n", (long)fp));
	result = input_new(&vtbl);
	this_thing = (input_crlf_ty *)result;
	this_thing->deeper = fp;
	this_thing->pos = 0;
	this_thing->delete_on_close = !!delete_on_close;
	this_thing->line_number = 0;
	this_thing->prev_was_newline = 1;
	this_thing->name_cache = 0;
	this_thing->escaped_newline = 0;
	trace(("return %08lX\n", (long)result));
	trace(("}\n"));
	return result;
}


void
input_crlf_escaped_newline(input_ty *ip)
{
	if (ip->vptr == &vtbl)
	{
		input_crlf_ty	*this_thing;

		this_thing = (input_crlf_ty *)ip;
		this_thing->escaped_newline = 1;
	}
}
