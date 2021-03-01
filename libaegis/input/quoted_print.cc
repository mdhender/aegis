//
//	aegis - project change supervisor
//	Copyright (C) 2001-2006, 2008 Peter Miller
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
// From RFC 1521...
//
//	The Quoted-Printable encoding is intended to represent data
//	that largely consists of octets that correspond to printable
//	characters in the ASCII character set. It encodes the data in
//	such a way that the resulting octets are unlikely to be modified
//	by mail transport. If the data being encoded are mostly ASCII
//	text, the encoded form of the data remains largely recognizable
//	by humans. A body which is entirely ASCII may also be encoded
//	in Quoted-Printable to ensure the integrity of the data should
//	the message pass through a character- translating, and/or
//	line-wrapping gateway.
//
//	In this encoding, octets are to be represented as determined by
//	the following rules:
//
//	Rule #1: (General 8-bit representation) Any octet, except those
//	indicating a line break according to the newline convention
//	of the canonical (standard) form of the data being encoded,
//	may be represented by an "=" followed by a two digit hexadecimal
//	representation of the octet's value. The digits of the hexadecimal
//	alphabet, for this purpose, are "0123456789ABCDEF". Uppercase
//	letters must be used when sending hexadecimal data, though a
//	robust implementation may choose to recognize lowercase letters
//	on receipt. Thus, for example, the value 12 (ASCII form feed)
//	can be represented by "=0C", and the value 61 (ASCII EQUAL SIGN)
//	can be represented by "=3D". Except when the following rules
//	allow an alternative encoding, this rule is mandatory.
//
//	Rule #2: (Literal representation) Octets with decimal values of
//	33 through 60 inclusive, and 62 through 126, inclusive, MAY be
//	represented as the ASCII characters which correspond to those
//	octets (EXCLAMATION POINT through LESS THAN, and GREATER THAN
//	through TILDE, respectively).
//
//	Rule #3: (White Space): Octets with values of 9 and 32 MAY be
//	represented as ASCII TAB (HT) and SPACE characters, respectively,
//	but MUST NOT be so represented at the end of an encoded line. Any
//	TAB (HT) or SPACE characters on an encoded line MUST thus be
//	followed on that line by a printable character. In particular,
//	an "=" at the end of an encoded line, indicating a soft line
//	break (see rule #5) may follow one or more TAB (HT) or SPACE
//	characters. It follows that an octet with value 9 or 32 appearing
//	at the end of an encoded line must be represented according
//	to Rule #1. This rule is necessary because some MTAs (Message
//	Transport Agents, programs which transport messages from one
//	user to another, or perform a part of such transfers) are known
//	to pad lines of text with SPACEs, and others are known to remove
//	"white space" characters from the end of a line. Therefore, when
//	decoding a Quoted-Printable body, any trailing white space on
//	a line must be deleted, as it will necessarily have been added
//	by intermediate transport agents.
//
//	Rule #4 (Line Breaks): A line break in a text body, independent of
//	what its representation is following the canonical representation
//	of the data being encoded, must be represented by a (RFC 822)
//	line break, which is a CRLF sequence, in the Quoted-Printable
//	encoding. Since the canonical representation of types other than
//	text do not generally include the representation of line breaks,
//	no hard line breaks (i.e.  line breaks that are intended to
//	be meaningful and to be displayed to the user) should occur
//	in the quoted-printable encoding of such types. Of course,
//	occurrences of "=0D", "=0A", "0A=0D" and "=0D=0A" will eventually
//	be encountered. In general, however, base64 is preferred over
//	quoted-printable for binary data.
//
//	Note that many implementations may elect to encode the local
//	representation of various content types directly, as described
//	in Appendix G. In particular, this may apply to plain text
//	material on systems that use newline conventions other than
//	CRLF delimiters. Such an implementation is permissible, but the
//	generation of line breaks must be generalized to account for
//	the case where alternate representations of newline sequences
//	are used.
//
//	Rule #5 (Soft Line Breaks): The Quoted-Printable encoding REQUIRES
//	that encoded lines be no more than 76 characters long. If longer
//	lines are to be encoded with the Quoted-Printable encoding,
//	'soft' line breaks must be used. An equal sign as the last
//	character on a encoded line indicates such a non-significant
//	('soft') line break in the encoded text. Thus if the "raw"
//	form of the line is a single unencoded line that says:
//
//		Now's the time for all folk to come to the aid of their country.
//
//	This can be represented, in the Quoted-Printable encoding, as
//
//		Now's the time =
//		for all folk to come =
//		to the aid of their country.
//
//	This provides a mechanism with which long lines are encoded in
//	such a way as to be restored by the user agent. The 76 character
//	limit does not count the trailing CRLF, but counts all other
//	characters, including any equal signs.
//
//	Since the hyphen character ("-") is represented as itself
//	in the Quoted-Printable encoding, care must be taken, when
//	encapsulating a quoted-printable encoded body in a multipart
//	entity, to ensure that the encapsulation boundary does not
//	appear anywhere in the encoded body. (A good strategy is to
//	choose a boundary that includes a character sequence such as
//	"=_" which can never appear in a quoted- printable body. See
//	the definition of multipart messages later in this document.)
//
//	NOTE: The quoted-printable encoding represents something
//	of a compromise between readability and reliability in
//	transport. Bodies encoded with the quoted-printable encoding
//	will work reliably over most mail gateways, but may not
//	work perfectly over a few gateways, notably those involving
//	translation into EBCDIC. (In theory, an EBCDIC gateway could
//	decode a quoted-printable body and re-encode it using base64,
//	but such gateways do not yet exist.) A higher level of confidence
//	is offered by the base64 Content-Transfer-Encoding. A way to get
//	reasonably reliable transport through EBCDIC gateways is to also
//	quote the ASCII characters
//
//		!"#$@[\]^`{|}~
//
//	according to rule #1. See Appendix B for more information.
//
//	Because quoted-printable data is generally assumed to be line-
//	oriented, it is to be expected that the representation of
//	the breaks between the lines of quoted printable data may
//	be altered in transport, in the same manner that plain text
//	mail has always been altered in Internet mail when passing
//	between systems with differing newline conventions. If such
//	alterations are likely to constitute a corruption of the data,
//	it is probably more sensible to use the base64 encoding rather
//	than the quoted-printable encoding.
//
//	WARNING TO IMPLEMENTORS: If binary data are encoded in quoted-
//	printable, care must be taken to encode CR and LF characters as
//	"=0D" and "=0A", respectively. In particular, a CRLF sequence in
//	binary data should be encoded as "=0D=0A". Otherwise, if CRLF
//	were represented as a hard line break, it might be incorrectly
//	decoded on platforms with different line break conventions.
//
//	For formalists, the syntax of quoted-printable data is described
//	by the following grammar:
//
//	quoted-printable := ([*(ptext / SPACE / TAB) ptext] ["="] CRLF)
//		; Maximum line length of 76 characters excluding CRLF
//
//	ptext := octet /<any ASCII character except "=", SPACE, or TAB>
//		; characters not listed as "mail-safe" in Appendix B
//		; are also not recommended.
//
//	octet := "=" 2(DIGIT / "A" / "B" / "C" / "D" / "E" / "F")
//		; octet must be used for characters > 127, =, SPACE, or TAB,
//		; and is recommended for any characters not listed in
//		; Appendix B as "mail-safe".
//

#include <common/ac/string.h>

#include <common/mem.h>
#include <libaegis/input/quoted_print.h>


input_quoted_printable::~input_quoted_printable()
{
}


input_quoted_printable::input_quoted_printable(input &arg) :
    deeper(arg),
    eof(false),
    pos(0)
{
}


static int
hex(int c)
{
    switch (c)
    {
    default:
	return -1;

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
	return (c - '0');

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
	return (c - 'A' + 10);

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
	return (c - 'a' + 10);
    }
}


long
input_quoted_printable::read_inner(void *data, size_t len)
{
    if (eof)
	return 0;
    unsigned char *cp = (unsigned char *)data;
    unsigned char *end = cp + len;
    next_char:
    while (cp < end)
    {
	int c = deeper->getch();
	if (c < 0)
	{
	    eof = true;
	    break;
	}
	if (c == ' ' || c == '\t')
	{
	    static char	*buf;
	    static size_t bufmax;

	    //
	    // We are supposed to suppress white space on
	    // the ends of lines.  This is because some
	    // (non-unix, non-windows) mail transfer agents
	    // add extra white space on the ends of lines.
	    // (Our corresponding encoding escapes trailing
	    // spaces and tabs.)
	    //
	    size_t bufpos = 0;
	    for (;;)
	    {
		//
		// Stash this character (we may need it later)
		//
		if (bufpos >= bufmax)
		{
		    bufmax = bufmax * 2 + 8;
		    char *new_buf = new char [bufmax];
		    memcpy(new_buf, buf, bufpos);
		    delete [] buf;
		    buf = new_buf;
		}
		buf[bufpos++] = c;

		//
		// See what comes next.
		//
		c = deeper->getch();
		if (c < 0)
		    break;
		if (c == '\n')
		{
		    *cp++ = '\n';
		    goto next_char;
		}
		if (c != ' ' && c != '\t')
		{
		    deeper->ungetc(c);
		    break;
		}
	    }

	    //
	    // Put as many of the buffered characters into
	    // the output as possible.	This means we won't
	    // double handle them (actually, we would O(n**2)
	    // handle them).
	    //
	    size_t nchars = end - cp;
	    if (nchars > bufpos)
		nchars = bufpos;
	    memcpy(cp, buf, nchars);
	    cp += nchars;

	    //
	    // If there wasn't room, there is no help for it.
	    // We will have to give the rest of the buffered
	    // characters back.	 Hopefully next time will
	    // be big enough for all of them.  The
	    // pathological case required >16KB of spaces and
	    // tabs: unlikely.
	    //
	    while (bufpos > nchars)
	    {
		--bufpos;
		deeper->ungetc(buf[bufpos]);
	    }

	    //
	    // Don't fall into the next statement, but start
	    // this loop from the top.	(We could have run
	    // out of output buf).
	    //
	    continue;
	}

	//
	// If this isn't an escape sequence, return the literal
	// character.
	//
	if (c != '=')
	{
	    *cp++ = c;
	    continue;
	}

	//
	// Grab two hex digits.	 If they aren't hex digits,
	// it is a format error.
	//
	// Except for trailing white space; that we ignore.
	//
	c = deeper->getch();
	if (c < 0)
	    break;
	if (c == ' ' || c == '\t')
	{
	    for (;;)
	    {
		c = deeper->getch();
		if (c == '\n')
		    break;
		if (c != ' ' && c != '\t')
		{
		    deeper->fatal_error("quoted printable: invalid character");
		    // NOTREACHED
		}
	    }
	}
	if (c == '\n')
	    continue;
	int n1 = hex(c);
	if (n1 < 0)
	{
	    deeper->fatal_error("quoted printable: invalid hex character");
	    // NOTREACHED
	}
	c = deeper->getch();
	int n2 = hex(c);
	if (n2 < 0)
	{
	    deeper->fatal_error("quoted printable: invalid hex character");
	    // NOTREACHED
	}
	*cp++ = ((n1 << 4) | n2);
    }
    long nbytes = (cp - (unsigned char *)data);
    pos += nbytes;
    return nbytes;
}


long
input_quoted_printable::ftell_inner()
{
    return pos;
}


nstring
input_quoted_printable::name()
{
    return deeper->name();
}


long
input_quoted_printable::length()
{
    return -1;
}


void
input_quoted_printable::keepalive()
{
    deeper->keepalive();
}


bool
input_quoted_printable::is_remote()
    const
{
    return deeper->is_remote();
}
