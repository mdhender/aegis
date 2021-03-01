//
//	aegis - project change supervisor
//	Copyright (C) 2001-2008 Peter Miller
//	Copyright (C) 2008 Walter Franzini
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

#include <common/ac/string.h>
#include <common/ac/limits.h>

#include <common/error.h>
#include <libaegis/input/uudecode.h>
#include <common/nstring/accumulator.h>
#include <common/trace.h>

//
// The NOT_DECODABLE value is placed into the itab array to annotate
// characters which are not part of the encoding table.  It must be
// positive.  The values 0..63 indicate valid decode values.
//
#define NOT_DECODABLE 127

static char default_encoding_table[] =
	   " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

input_uudecode::~input_uudecode()
{
}


input_uudecode::input_uudecode(input &arg) :
    deeper(arg),
    pos(0),
    state(0),
    checksum(-1),
    padding(-1)
{
}


long
input_uudecode::read_inner(void *data, size_t len)
{
    //
    // Look for the line which starts with 'begin '
    //
    if (state >= 6)
	return 0;
    unsigned char *cp = (unsigned char *)data;
    unsigned char *end = cp + len;
    assert(len >= 64);
    if (len < 64)
	deeper->fatal_error("uudecode: read too little");
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
	    int c = deeper->getch();
	    if (c < 0)
	    {
		if (bp == buf && state == 0)
		{
		    deeper->fatal_error("uudecode: no data in file");
		}
		else
		{
		    deeper->fatal_error("uudecode: premature end-of-file");
		}
	    }
	    if (c == '\n')
		break;
	    if (bp >= ENDOF(buf) - 1)
	    {
		deeper->fatal_error("uudecode: line too long");
	    }
	    *bp++ = c;
	}
	*bp = 0;

	ilen = bp - buf;
	ep = bp;
	bp = buf;

	switch (state)
	{
	case 0:
	    // before "table" or "begin "
	    if (!strcmp((const char *)buf, "table"))
		state = 1;
	    else if (!memcmp(buf, "begin ", 6))
	    {
		state = 4;

		//
		// use default encoding
		//
		memcpy(etab, default_encoding_table, 64);

		//
		// invert the encoding table
		//
		invert:
		for (unsigned int k = 0; k <= UCHAR_MAX; ++k)
		    itab[k] = NOT_DECODABLE;
		for (int j = 0; j < 64; ++j)
		{
		    n = (unsigned char)etab[j];
		    if (itab[n] != NOT_DECODABLE)
		    {
			deeper->fatal_error("uudecode: table has duplicate");
		    }
		    itab[n] = j;
		}

		//
		// Hack to cope with common encoding
		// extension (because many e-mail
		// forwarders rip of trailing spaces).
		//
		if (etab[0] == ' ' && itab[(unsigned char)'`'] == NOT_DECODABLE)
		    itab[(unsigned char)'`'] = 0;
	    }
	    continue;

	case 1:
	    // after "table"
	    if (ilen != 32)
	    {
		deeper->fatal_error("uudecode: broken table section");
	    }
	    memcpy(etab, buf, 32);
	    state++;
	    continue;

	case 2:
	    // after "table" + 1
	    if (ilen != 32)
	    {
		deeper->fatal_error("uudecode: broken table section");
	    }
	    memcpy(etab + 32, buf, 32);
	    state++;
	    goto invert;

	case 3:
	    // after "table" + 2
	    if (!ilen)
		continue;
	    if (!memcmp(buf, "begin ", 6))
	    {
		state++;
	    }
	    else
	    {
		deeper->fatal_error("uudecode: expected \"begin\"");
	    }
	    continue;

	case 4:
	    // data section
	    if (!ilen)
	    {
		deeper->fatal_error("uudecode: blank line in data section");
	    }

	    //
	    // get length
	    //
	    n = itab[*bp++];
	    if (n == NOT_DECODABLE)
	    {
		broken:
		deeper->fatal_error
	       	(
		    "uudecode: broken data section (character not in encoding "
			"table)"
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
		    if (checksum < 0)
			checksum = 1;
		    break;

		case 1:
		    if (padding < 0)
			padding = 1;
		    if (checksum < 0)
			checksum = 0;
		    break;

		default:
		    broken_length:
		    deeper->fatal_error("uudecode: data line has wrong length");
		}
		check_the_sum = checksum;
		break;

	    case 1:
		switch ((n + 2) / 3 * 4 + 2 - ilen)
		{
		case 0:
		    if (padding < 0)
			padding = 1;
		    if (checksum < 0)
			checksum = 1;
		    break;

		case 1:
		    if (padding < 0)
			padding = 1;
		    if (checksum < 0)
			checksum = 0;
		    break;

		case 2:
		    if (padding < 0)
			padding = 0;
		    if (checksum < 0)
			checksum = 1;
		    break;

		case 3:
		    if (padding < 0)
			padding = 0;
		    if (checksum < 0)
			checksum = 0;
		    break;

		default:
		    goto broken_length;
		}
		check_the_sum = checksum;
		break;

	    case 2:
		switch ((n + 2) / 3 * 4 + 2 - ilen)
		{
		case 0:
		    if (padding < 0)
			padding = 1;
		    if (checksum < 0)
			checksum = 1;
		    break;

		case 1:
		    // padding or checksum, but not both
		    if (checksum >= 0)
			padding = !checksum;
		    break;

		case 2:
		    if (padding < 0)
			padding = 0;
		    if (checksum < 0)
			checksum = 0;
		    break;

		default:
		    goto broken_length;
		}
		check_the_sum =
                    checksum && padding >= 0;
		break;
	    }

	    //
	    // read the bytes
	    //
	    sum = n;
	    for (int j = 0; j < n; j += 3)
	    {
		int c1 = itab[*bp++];
		if (c1 == NOT_DECODABLE)
		    goto broken;
		assert(c1 < 64);
		sum += c1;

		int c2 = itab[*bp++];
		if (c2 == NOT_DECODABLE)
		    goto broken;
		assert(c2 < 64);
		sum += c2;

		int c = (c1 << 2) | ((c2 >> 4) & 0x03);
		*cp++ = c;

		if (j + 1 < n || padding == 1)
		{
		    int c3 = itab[*bp++];
		    if (c3 == NOT_DECODABLE)
			goto broken;
		    assert(c3 < 64);
		    sum += c3;

		    if (j + 1 < n)
		    {
			c = ((c2 << 4) & 0xF0) | ((c3 >> 2) & 0x0F);
			*cp++ = c;
		    }

		    if (j + 2 < n || padding == 1)
		    {
			int c4 = itab[*bp++];
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
		    deeper->fatal_error("uudecode: checksum required");
		}
		int c = itab[*bp++];
		if (c == NOT_DECODABLE)
		    goto broken;
		assert(c < 64);
		if (c != (sum & 077))
		{
		    deeper->fatal_error("uudecode: checksum is wrong");
		}
	    }
	    if (n == 0)
		state++;
	    continue;

	case 5:
	    if (!strcmp((const char *)buf, "end"))
	    {
		state++;
		break;
	    }
	    deeper->fatal_error("uudecode: \"end\" required");

	case 6:
	    break;
	}
	break;
    }

    size_t nbytes = (cp - (unsigned char *)data);
    pos += nbytes;
    return nbytes;
}


long
input_uudecode::ftell_inner()
{
    return pos;
}


nstring
input_uudecode::name()
{
    return deeper->name();
}


long
input_uudecode::length()
{
    return -1;
}


void
input_uudecode::keepalive()
{
    deeper->keepalive();
}


bool
input_uudecode::candidate(input &ifp)
{
    trace(("input_uudecode_recognise()\n{\n"));
    static char magic[] = "begin ";
    bool result = 0;
    nstring_accumulator sac;
    int char_state = 0;
    while (sac.size() < 8000)
    {
	int c = ifp->getch();
	if (c < 0)
	    break;
	sac.push_back(c);
	if (c == '\n')
	    char_state = 0;
	else if ((size_t)char_state < sizeof(magic) && c == magic[char_state])
	{
	    ++char_state;
	    if (!magic[char_state])
	    {
		result = 1;
		break;
	    }
	}
	else
	    char_state = 666;
    }
    ifp->unread(sac.get_data(), sac.size());
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}


bool
input_uudecode::is_remote()
    const
{
    return deeper->is_remote();
}
