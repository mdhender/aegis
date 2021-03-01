//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <libaegis/input/base64.h>
#include <common/stracc.h>


input_base64::~input_base64()
{
}


input_base64::input_base64(input &arg) :
    deeper(arg),
    pos(0),
    residual_bits(0),
    residual_value(0),
    eof(false)
{
}


long
input_base64::read_inner(void *data, size_t len)
{
    if (eof)
	return 0;
    unsigned char *cp = (unsigned char *)data;
    unsigned char *end = cp + len;
    while (cp < end)
    {
	while (residual_bits < 8)
	{
	    int c = deeper->getch();
	    switch (c)
	    {
	    case ' ':
	    case '\t':
	    case '\r':
	    case '\n':
		// The RFC says to ignore white space
		continue;

	    case '=':
		eof = 1;
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
		//
		// This next statement is not portable to
		// non-ascii character sets, because A-Z
		// are not guaranteed to be continuous.
		//
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
		//
		// This next statement is not portable to
		// non-ascii character sets, because a-z
		// are not guaranteed to be continuous.
		//
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

	    case '#':
		//
		// The RFC says we have the option of ignoring bogus
		// characters or whinging about them.  If we ignore the
		// '#' symbol, we can decode the patch meta data without
		// an extra prefix removing input filter.
		//
		continue;

	    default:
		if (c < 0)
		{
		    if (residual_bits != 0)
			fatal_error("base64: residual bits != 0");
		    eof = 1;
		    goto done;
		}
		fatal_error("base64: invalid character");
		// NOTREACHED
	    }
	    residual_value = (residual_value << 6) + c;
	    residual_bits += 6;
	}
	residual_bits -= 8;
	*cp++ = (residual_value >> residual_bits);
    }
    done:
    size_t nbytes = (cp - (unsigned char *)data);
    pos += nbytes;
    return nbytes;
}


long
input_base64::ftell_inner()
{
    return pos;
}


nstring
input_base64::name()
{
    return deeper->name();
}


long
input_base64::length()
{
    return -1;
}


void
input_base64::keepalive()
{
    deeper->keepalive();
}


bool
input_base64::recognise(input &ifp)
{
    //
    // There are only a few characters which are acceptable to
    // the base64 filter.  Any others are conclusive evidence
    // of wrongness.
    //
    bool result = true;
    stracc_t sac;
    while (sac.size() < 8000)
    {
	int c = ifp->getch();
	if (c < 0)
	    break;
	sac.push_back(c);
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
	    result = false;
	    break;
	}
	break;
    }
    ifp->unread(sac.get_data(), sac.size());
    return result;
}


bool
input_base64::is_remote()
    const
{
    return deeper->is_remote();
}
