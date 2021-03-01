//
//	aegis - project change supervisor
//	Copyright (C) 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate uudecodes
//

#include <ac/string.h>
#include <ac/limits.h>

#include <error.h>
#include <input/uudecode.h>
#include <input/private.h>
#include <stracc.h>
#include <trace.h>

//
// The NOT_DECODABLE value is placed into the itab array to annotate
// characters which are not part of the encoding table.  It must be
// positive.  The values 0..63 indicate valid decode values.
//
#define NOT_DECODABLE 127

static char default_encoding_table[] =
	   " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

struct input_uudecode_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
    int		    close_on_close;
    long	    pos;
    int		    state;
    char	    etab[64];
    unsigned char   itab[UCHAR_MAX + 1];
    int		    checksum;
    int		    padding;
};


static void
input_uudecode_destructor(input_ty *fp)
{
    input_uudecode_ty *this_thing;

    this_thing = (input_uudecode_ty *)fp;
    if (this_thing->close_on_close)
	input_delete(this_thing->deeper);
    this_thing->deeper = 0; // paranoia
}


static long
input_uudecode_read(input_ty *fp, void *data, size_t len)
{
    input_uudecode_ty *this_thing;
    int		    c;
    unsigned char   *cp;
    unsigned char   *end;
    size_t	    nbytes;
    int		    j;

    //
    // Look for the line which starts with 'begin '
    //
    this_thing = (input_uudecode_ty *)fp;
    if (this_thing->state >= 6)
	return 0;
    cp = (unsigned char *)data;
    end = cp + len;
    assert(len >= 64);
    if (len < 64)
	input_fatal_error(this_thing->deeper, "uudecode: read too little");
    while (cp + 64 < end)
    {
	unsigned char	buf[1000];
	int		ilen;
	unsigned char	*bp;
	unsigned char	*ep;
	int		sum;
	int		n;
	int		check_the_sum;

	//
	// read a line
	//
	bp = buf;
	for (;;)
	{
	    c = input_getc(this_thing->deeper);
	    if (c < 0)
	    {
		if (bp == buf && this_thing->state == 0)
		{
		    input_fatal_error
		    (
			this_thing->deeper,
			"uudecode: no data in file"
		    );
		}
		else
		{
		    input_fatal_error(this_thing->deeper,
                                      "premature end-of-file");
		}
	    }
	    if (c == '\n')
		break;
	    if (bp >= ENDOF(buf) - 1)
	    {
		input_fatal_error(this_thing->deeper, "line too long");
	    }
	    *bp++ = c;
	}
	*bp = 0;

	ilen = bp - buf;
	ep = bp;
	bp = buf;

	switch (this_thing->state)
	{
	case 0:
	    // before "table" or "begin "
	    if (!strcmp((const char *)buf, "table"))
		this_thing->state = 1;
	    else if (!memcmp(buf, "begin ", 6))
	    {
		this_thing->state = 4;

		//
		// use default encoding
		//
		memcpy(this_thing->etab, default_encoding_table, 64);

		//
		// invert the encoding table
		//
		invert:
		for (j = 0; j <= UCHAR_MAX; ++j)
		    this_thing->itab[j] = NOT_DECODABLE;
		for (j = 0; j < 64; ++j)
		{
		    n = (unsigned char)this_thing->etab[j];
		    if (this_thing->itab[n] != NOT_DECODABLE)
		    {
			input_fatal_error(this_thing->deeper,
                                          "table has duplicate");
		    }
		    this_thing->itab[n] = j;
		}

		//
		// Hack to cope with common encoding
		// extension (because many e-mail
		// forwarders rip of trailing spaces).
		//
		if (this_thing->etab[0] == ' ' &&
                    this_thing->itab['`'] == NOT_DECODABLE)
		    this_thing->itab['`'] = 0;
	    }
	    continue;

	case 1:
	    // after "table"
	    if (ilen != 32)
	    {
		input_fatal_error(this_thing->deeper, "broken table section");
	    }
	    memcpy(this_thing->etab, buf, 32);
	    this_thing->state++;
	    continue;

	case 2:
	    // after "table" + 1
	    if (ilen != 32)
	    {
		input_fatal_error(this_thing->deeper, "broken table section");
	    }
	    memcpy(this_thing->etab + 32, buf, 32);
	    this_thing->state++;
	    goto invert;

	case 3:
	    // after "table" + 2
	    if (!ilen)
		continue;
	    if (!memcmp(buf, "begin ", 6))
		this_thing->state++;
	    else
	    {
		input_fatal_error(this_thing->deeper, "expected \"begin\"");
	    }
	    continue;

	case 4:
	    // data section
	    if (!ilen)
	    {
		input_fatal_error(this_thing->deeper,
                                  "blank line in data section");
	    }

	    //
	    // get length
	    //
	    n = this_thing->itab[*bp++];
	    if (n == NOT_DECODABLE)
	    {
		broken:
		input_fatal_error
		(
		    this_thing->deeper,
		    "broken data section (character not in encoding table)"
		);
	    }
	    assert(n < 64);

	    check_the_sum = 0;
	    switch (n % 3)
	    {
	    case 0:
		switch ((n * 4 + 2) / 3 + 2 - ilen)
		{
		case 0:
		    if (this_thing->checksum < 0)
			this_thing->checksum = 1;
		    break;

		case 1:
		    if (this_thing->padding < 0)
			this_thing->padding = 1;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 0;
		    break;

		default:
		    broken_length:
		    input_fatal_error
		    (
			this_thing->deeper,
			"data line has wrong length"
		    );
		}
		check_the_sum = this_thing->checksum;
		break;

	    case 1:
		switch ((n + 2) / 3 * 4 + 2 - ilen)
		{
		case 0:
		    if (this_thing->padding < 0)
			this_thing->padding = 1;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 1;
		    break;

		case 1:
		    if (this_thing->padding < 0)
			this_thing->padding = 1;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 0;
		    break;

		case 2:
		    if (this_thing->padding < 0)
			this_thing->padding = 0;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 1;
		    break;

		case 3:
		    if (this_thing->padding < 0)
			this_thing->padding = 0;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 0;
		    break;

		default:
		    goto broken_length;
		}
		check_the_sum = this_thing->checksum;
		break;

	    case 2:
		switch ((n + 2) / 3 * 4 + 2 - ilen)
		{
		case 0:
		    if (this_thing->padding < 0)
			this_thing->padding = 1;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 1;
		    break;

		case 1:
		    // padding or checksum, but not both
		    if (this_thing->checksum >= 0)
			this_thing->padding = !this_thing->checksum;
		    break;

		case 2:
		    if (this_thing->padding < 0)
			this_thing->padding = 0;
		    if (this_thing->checksum < 0)
			this_thing->checksum = 0;
		    break;

		default:
		    goto broken_length;
		}
		check_the_sum =
                    this_thing->checksum && this_thing->padding >= 0;
		break;
	    }

	    //
	    // read the bytes
	    //
	    sum = n;
	    for (j = 0; j < n; j += 3)
	    {
		int		c1;
		int		c2;
		int		c3;
		int		c4;

		c1 = this_thing->itab[*bp++];
		if (c1 == NOT_DECODABLE)
		    goto broken;
		assert(c1 < 64);
		sum += c1;

		c2 = this_thing->itab[*bp++];
		if (c2 == NOT_DECODABLE)
		    goto broken;
		assert(c2 < 64);
		sum += c2;

		c = (c1 << 2) | ((c2 >> 4) & 0x03);
		*cp++ = c;

		if (j + 1 < n || this_thing->padding == 1)
		{
		    c3 = this_thing->itab[*bp++];
		    if (c3 == NOT_DECODABLE)
			goto broken;
		    assert(c3 < 64);
		    sum += c3;

		    if (j + 1 < n)
		    {
			c = ((c2 << 4) & 0xF0) | ((c3 >> 2) & 0x0F);
			*cp++ = c;
		    }

		    if (j + 2 < n || this_thing->padding == 1)
		    {
			c4 = this_thing->itab[*bp++];
			if (c4 == NOT_DECODABLE)
			    goto broken;
			assert(c4 < 64);
			sum += c4;

			if (j + 2 < n)
			{
			    c = ((c3 << 6) & 0xC0) | c4;
			    *cp++ = c;
			}
		    }
		}
	    }
	    if (check_the_sum)
	    {
		if (bp >= ep)
		{
		    input_fatal_error(this_thing->deeper, "checksum required");
		}
		c = this_thing->itab[*bp++];
		if (c == NOT_DECODABLE)
		    goto broken;
		assert(c < 64);
		if (c != (sum & 077))
		{
		    input_fatal_error(this_thing->deeper, "checksum is wrong");
		}
	    }
	    if (n == 0)
		this_thing->state++;
	    continue;

	case 5:
	    if (!strcmp((const char *)buf, "end"))
	    {
		this_thing->state++;
		break;
	    }
	    input_fatal_error(this_thing->deeper, "end required");

	case 6:
	    break;
	}
	break;
    }

    nbytes = (cp - (unsigned char *)data);
    this_thing->pos += nbytes;
    return nbytes;
}


static long
input_uudecode_tell(input_ty *deeper)
{
    input_uudecode_ty *this_thing;

    this_thing = (input_uudecode_ty *)deeper;
    return this_thing->pos;
}


static struct string_ty *
input_uudecode_name(input_ty *fp)
{
    input_uudecode_ty *this_thing;

    this_thing = (input_uudecode_ty *)fp;
    return input_name(this_thing->deeper);
}


static long
input_uudecode_length(input_ty *fp)
{
    return -1;
}


static void
input_uudecode_keepalive(input_ty *fp)
{
    input_uudecode_ty *ip;

    ip = (input_uudecode_ty *)fp;
    input_keepalive(ip->deeper);
}


static input_vtbl_ty vtbl =
{
    sizeof(input_uudecode_ty),
    input_uudecode_destructor,
    input_uudecode_read,
    input_uudecode_tell,
    input_uudecode_name,
    input_uudecode_length,
    input_uudecode_keepalive,
};


input_ty *
input_uudecode(input_ty *deeper, int coc)
{
    input_ty	    *result;
    input_uudecode_ty *this_thing;

    result = input_new(&vtbl);
    this_thing = (input_uudecode_ty *)result;
    this_thing->deeper = deeper;
    this_thing->close_on_close = !!coc;
    this_thing->pos = 0;
    this_thing->state = 0;
    this_thing->checksum = -1;
    this_thing->padding = -1;
    return result;
}


int
input_uudecode_recognise(input_ty *ifp)
{
    static char	    magic[] = "begin ";
    int		    result;
    int		    c;
    stracc_t	    buffer;
    int		    state;

    trace(("input_uudecode_recognise(ifp = %08lX)\n{\n", (long)ifp));
    result = 0;
    stracc_constructor(&buffer);
    stracc_open(&buffer);
    state = 0;
    while (buffer.length < 8000)
    {
	c = input_getc(ifp);
	if (c < 0)
	    break;
	stracc_char(&buffer, c);
	if (c == '\n')
	    state = 0;
	else if ((size_t)state < sizeof(magic) && c == magic[state])
	{
	    ++state;
	    if (!magic[state])
	    {
		result = 1;
		break;
	    }
	}
	else
	    state = 666;
    }
    input_unread(ifp, buffer.buffer, buffer.length);
    stracc_destructor(&buffer);
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}
