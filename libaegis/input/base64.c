/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001-2003 Peter Miller;
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
#include <stracc.h>


typedef struct input_base64_ty input_base64_ty;
struct input_base64_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
    int		    close_on_close;
    long	    pos;
    int		    residual_bits;
    int		    residual_value;
    int		    eof;
};


static void
input_base64_destructor(input_ty *fp)
{
    input_base64_ty *this_thing;

    this_thing = (input_base64_ty *)fp;
    if (this_thing->close_on_close)
	input_delete(this_thing->deeper);
    this_thing->deeper = 0; /* paranoia */
}


static long
input_base64_read(input_ty *fp, void *data, size_t len)
{
    input_base64_ty *this_thing;
    int		    c;
    unsigned char   *cp;
    unsigned char   *end;
    size_t	    nbytes;

    this_thing = (input_base64_ty *)fp;
    if (this_thing->eof)
	return 0;
    cp = (unsigned char *)data;
    end = cp + len;
    while (cp < end)
    {
	while (this_thing->residual_bits < 8)
	{
	    c = input_getc(this_thing->deeper);
	    switch (c)
	    {
	    case ' ':
	    case '\t':
	    case '\r':
	    case '\n':
		continue;

	    case '=':
		this_thing->eof = 1;
		goto done;

	    case 'A':
	    case 'B':
	    case 'C':
	    case 'D':
	    case 'E':
	    case 'F':
	    case 'G':
	    case 'H':
	    case 'I':
	    case 'J':
	    case 'K':
	    case 'L':
	    case 'M':
	    case 'N':
	    case 'O':
	    case 'P':
	    case 'Q':
	    case 'R':
	    case 'S':
	    case 'T':
	    case 'U':
	    case 'V':
	    case 'W':
	    case 'X':
	    case 'Y':
	    case 'Z':
		/*
		 * This next statement is not portable to
		 * non-ascii character sets, because A-Z
		 * are not guaranteed to be continuous.
		 */
		c = c - 'A';
		break;

	    case 'a':
	    case 'b':
	    case 'c':
	    case 'd':
	    case 'e':
	    case 'f':
	    case 'g':
	    case 'h':
	    case 'i':
	    case 'j':
	    case 'k':
	    case 'l':
	    case 'm':
	    case 'n':
	    case 'o':
	    case 'p':
	    case 'q':
	    case 'r':
	    case 's':
	    case 't':
	    case 'u':
	    case 'v':
	    case 'w':
	    case 'x':
	    case 'y':
	    case 'z':
		/*
		 * This next statement is not portable to
		 * non-ascii character sets, because a-z
		 * are not guaranteed to be continuous.
		 */
		c = c - 'a' + 26;
		break;

	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
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
		    if (this_thing->residual_bits != 0)
			input_fatal_error(fp, "base64: residual bits != 0");
		    this_thing->eof = 1;
		    goto done;
		}
		input_fatal_error(fp, "base64: invalid character");
		/* NOTREACHED */
	    }
	    this_thing->residual_value = (this_thing->residual_value << 6) + c;
	    this_thing->residual_bits += 6;
	}
	this_thing->residual_bits -= 8;
	*cp++ = (this_thing->residual_value >> this_thing->residual_bits);
    }
    done:
    nbytes = (cp - (unsigned char *)data);
    this_thing->pos += nbytes;
    return nbytes;
}


static long
input_base64_tell(input_ty *deeper)
{
    input_base64_ty *this_thing;

    this_thing = (input_base64_ty *)deeper;
    return this_thing->pos;
}


static struct string_ty *
input_base64_name(input_ty *fp)
{
    input_base64_ty *this_thing;

    this_thing = (input_base64_ty *)fp;
    return input_name(this_thing->deeper);
}


static long
input_base64_length(input_ty *fp)
{
    return -1;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_base64_ty),
    input_base64_destructor,
    input_base64_read,
    input_base64_tell,
    input_base64_name,
    input_base64_length,
};


input_ty *
input_base64(input_ty *deeper, int coc)
{
    input_ty	    *result;
    input_base64_ty *this_thing;

    result = input_new(&vtbl);
    this_thing = (input_base64_ty *)result;
    this_thing->deeper = deeper;
    this_thing->close_on_close = !!coc;
    this_thing->pos = 0;
    this_thing->residual_bits = 0;
    this_thing->residual_value = 0;
    this_thing->eof = 0;
    return result;
}


int
input_base64_recognise(input_ty *ifp)
{
    int		    result;
    int		    c;
    stracc_t	    buffer;

    /*
     * There are only a few characters which ara acceptable to
     * the base64 filter.  Any others are conclusive evidence
     * of wrongness.
     */
    result = 1;
    stracc_constructor(&buffer);
    stracc_open(&buffer);
    while (buffer.length < 8000)
    {
	c = input_getc(ifp);
	if (c < 0)
	    break;
	stracc_char(&buffer, c);
	switch (c)
	{
	case '\t':
	case '\n':
	case '\r':
	case ' ':
	case '+':
	case '/':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '=':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	    continue;

	default:
	    result = 0;
	    break;
	}
	break;
    }
    input_unread(ifp, buffer.buffer, buffer.length);
    stracc_destructor(&buffer);
    return result;
}
