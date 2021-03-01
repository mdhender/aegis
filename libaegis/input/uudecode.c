/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate uudecodes
 */

#include <ac/string.h>

#include <error.h>
#include <input/uudecode.h>
#include <input/private.h>
#include <stracc.h>
#include <trace.h>


static char default_encoding_table[] =
	   " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

typedef struct input_uudecode_ty input_uudecode_ty;
struct input_uudecode_ty
{
    input_ty	    inherited;
    input_ty	    *deeper;
    int		    close_on_close;
    long	    pos;
    int		    state;
    char	    etab[64];
    char	    itab[256];
    int		    checksum;
    int		    padding;
};


static void input_uudecode_destructor _((input_ty *));

static void
input_uudecode_destructor(fp)
    input_ty	    *fp;
{
    input_uudecode_ty *this;

    this = (input_uudecode_ty *)fp;
    if (this->close_on_close)
	input_delete(this->deeper);
    this->deeper = 0; /* paranoia */
}


static long input_uudecode_read _((input_ty *, void *, size_t));

static long
input_uudecode_read(fp, data, len)
    input_ty	    *fp;
    void	    *data;
    size_t	    len;
{
    input_uudecode_ty *this;
    int		    c;
    unsigned char   *cp;
    unsigned char   *end;
    size_t	    nbytes;
    int		    j;

    /*
     * Look for the line which starts with 'begin '
     */
    this = (input_uudecode_ty *)fp;
    if (this->state >= 6)
	return 0;
    cp = data;
    end = cp + len;
    assert(len>=64);
    if (len < 64)
	input_fatal_error(this->deeper, "uudecode: read too little");
    while (cp + 64 < end)
    {
	unsigned char	buf[1000];
	int		ilen;
	unsigned char	*bp;
	unsigned char	*ep;
	int		sum;
	int		n;
	int		check_the_sum;

	/*
	 * read a line
	 */
	bp = buf;
	for (;;)
	{
	    c = input_getc(this->deeper);
	    if (c < 0)
	    {
		if (bp == buf && this->state == 0)
		{
		    input_fatal_error
		    (
			this->deeper,
			"uudecode: no data in file"
		    );
		}
		else
		{
		    input_fatal_error(this->deeper, "premature end-of-file");
		}
	    }
	    if (c == '\n')
		break;
	    if (bp >= ENDOF(buf) - 1)
	    {
		input_fatal_error(this->deeper, "line too long");
	    }
	    *bp++ = c;
	}
	*bp = 0;

	ilen = bp - buf;
	ep = bp;
	bp = buf;

	switch (this->state)
	{
	case 0:
	    /* before "table" or "begin " */
	    if (!strcmp(buf, "table"))
		this->state = 1;
	    else if (!memcmp(buf, "begin ", 6))
	    {
		this->state = 4;

		/*
		 * use default encoding
		 */
		memcpy(this->etab, default_encoding_table, 64);

		/*
		 * invert the encoding table
		 */
		invert:
		for (j = 0; j < 256; ++j)
		    this->itab[j] = -1;
		for (j = 0; j < 64; ++j)
		{
		    n = (unsigned char)this->etab[j];
		    if (this->itab[n] >= 0)
		    {
			input_fatal_error(this->deeper, "table has duplicate");
		    }
		    this->itab[n] = j;
		}

		/*
		 * Hack to cope with common encoding
		 * extension (because many e-mail
		 * forwarders rip of trailing spaces).
		 */
		if (this->etab[0] == ' ' && this->itab['`'] < 0)
		    this->itab['`'] = 0;
	    }
	    continue;

	case 1:
	    /* after "table" */
	    if (ilen != 32)
	    {
		input_fatal_error(this->deeper, "broken table section");
	    }
	    memcpy(this->etab, buf, 32);
	    this->state++;
	    continue;

	case 2:
	    /* after "table" + 1 */
	    if (ilen != 32)
	    {
		input_fatal_error(this->deeper, "broken table section");
	    }
	    memcpy(this->etab + 32, buf, 32);
	    this->state++;
	    goto invert;

	case 3:
	    /* after "table" + 2 */
	    if (!ilen)
		continue;
	    if (!memcmp(buf, "begin ", 6))
		this->state++;
	    else
	    {
		input_fatal_error(this->deeper, "expected \"begin\"");
	    }
	    continue;

	case 4:
	    /* data section */
	    if (!ilen)
	    {
		input_fatal_error(this->deeper, "blank line in data section");
	    }

	    /*
	     * get length
	     */
	    n = this->itab[*bp++];
	    if (n < 0)
	    {
		broken:
		input_fatal_error
		(
		    this->deeper,
		    "broken data section (character not in encoding table)"
		);
	    }
	    assert(n<64);

	    check_the_sum = 0;
	    switch (n % 3)
	    {
	    case 0:
		switch ((n * 4 + 2) / 3 + 2 - ilen)
		{
		case 0:
		    if (this->checksum < 0)
			this->checksum = 1;
		    break;

		case 1:
		    if (this->padding < 0)
			this->padding = 1;
		    if (this->checksum < 0)
			this->checksum = 0;
		    break;

		default:
		    broken_length:
		    input_fatal_error
		    (
			this->deeper,
			"data line has wrong length"
		    );
		}
		check_the_sum = this->checksum;
		break;

	    case 1:
		switch ((n + 2) / 3 * 4 + 2 - ilen)
		{
		case 0:
		    if (this->padding < 0)
			this->padding = 1;
		    if (this->checksum < 0)
			this->checksum = 1;
		    break;

		case 1:
		    if (this->padding < 0)
			this->padding = 1;
		    if (this->checksum < 0)
			this->checksum = 0;
		    break;

		case 2:
		    if (this->padding < 0)
			this->padding = 0;
		    if (this->checksum < 0)
			this->checksum = 1;
		    break;

		case 3:
		    if (this->padding < 0)
			this->padding = 0;
		    if (this->checksum < 0)
			this->checksum = 0;
		    break;

		default:
		    goto broken_length;
		}
		check_the_sum = this->checksum;
		break;

	    case 2:
		switch ((n + 2) / 3 * 4 + 2 - ilen)
		{
		case 0:
		    if (this->padding < 0)
			this->padding = 1;
		    if (this->checksum < 0)
			this->checksum = 1;
		    break;

		case 1:
		    /* padding or checksum, but not both */
		    if (this->checksum >= 0)
			this->padding = !this->checksum;
		    break;

		case 2:
		    if (this->padding < 0)
			this->padding = 0;
		    if (this->checksum < 0)
			this->checksum = 0;
		    break;

		default:
		    goto broken_length;
		}
		check_the_sum = this->checksum && this->padding >= 0;
		break;
	    }

	    /*
	     * read the bytes
	     */
	    sum = n;
	    for (j = 0; j < n; j += 3)
	    {
		int		c1;
		int		c2;
		int		c3;
		int		c4;

		c1 = this->itab[*bp++];
		if (c1 < 0)
		    goto broken;
		sum += c1;

		c2 = this->itab[*bp++];
		if (c2 < 0)
		    goto broken;
		sum += c2;

		c = (c1 << 2) | ((c2 >> 4) & 0x03);
		*cp++ = c;

		if (j + 1 < n || this->padding == 1)
		{
		    c3 = this->itab[*bp++];
		    if (c3 < 0)
			goto broken;
		    sum += c3;

		    if (j + 1 < n)
		    {
			c = ((c2 << 4) & 0xF0) | ((c3 >> 2) & 0x0F);
			*cp++ = c;
		    }

		    if (j + 2 < n || this->padding == 1)
		    {
			c4 = this->itab[*bp++];
			if (c4 < 0)
			    goto broken;
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
		    input_fatal_error(this->deeper, "checksum required");
		}
		c = this->itab[*bp++];
		if (c < 0)
		    goto broken;
		if (c != (sum & 077))
		{
		    input_fatal_error(this->deeper, "checksum is wrong");
		}
	    }
	    if (n == 0)
		this->state++;
	    continue;

	case 5:
	    if (!strcmp(buf, "end"))
	    {
		this->state++;
		break;
	    }
	    input_fatal_error(this->deeper, "end required");

	case 6:
	    break;
	}
	break;
    }

    nbytes = (cp - (unsigned char *)data);
    this->pos += nbytes;
    return nbytes;
}


static long input_uudecode_tell _((input_ty *));

static long
input_uudecode_tell(deeper)
    input_ty	    *deeper;
{
    input_uudecode_ty *this;

    this = (input_uudecode_ty *)deeper;
    return this->pos;
}


static struct string_ty *input_uudecode_name _((input_ty *));

static struct string_ty *
input_uudecode_name(fp)
    input_ty	    *fp;
{
    input_uudecode_ty *this;

    this = (input_uudecode_ty *)fp;
    return input_name(this->deeper);
}


static long input_uudecode_length _((input_ty *));

static long
input_uudecode_length(fp)
    input_ty	    *fp;
{
    return -1;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_uudecode_ty),
    input_uudecode_destructor,
    input_uudecode_read,
    input_uudecode_tell,
    input_uudecode_name,
    input_uudecode_length,
};


input_ty *
input_uudecode(deeper, coc)
    input_ty	    *deeper;
    int		    coc;
{
    input_ty	    *result;
    input_uudecode_ty *this;

    result = input_new(&vtbl);
    this = (input_uudecode_ty *)result;
    this->deeper = deeper;
    this->close_on_close = !!coc;
    this->pos = 0;
    this->state = 0;
    this->checksum = -1;
    this->padding = -1;
    return result;
}


int
input_uudecode_recognise(ifp)
    input_ty	    *ifp;
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
	else if (state < sizeof(magic) && c == magic[state])
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
