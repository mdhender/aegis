//
// aegis - project change supervisor
// Copyright (C) 2001-2006, 2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// From RFC 1521...
//
//      The Quoted-Printable encoding is intended to represent data
//      that largely consists of octets that correspond to printable
//      characters in the ASCII character set. It encodes the data in
//      such a way that the resulting octets are unlikely to be modified
//      by mail transport. If the data being encoded are mostly ASCII
//      text, the encoded form of the data remains largely recognizable
//      by humans. A body which is entirely ASCII may also be encoded
//      in Quoted-Printable to ensure the integrity of the data should
//      the message pass through a character- translating, and/or
//      line-wrapping gateway.
//
//      In this encoding, octets are to be represented as determined by
//      the following rules:
//
//      Rule #1: (General 8-bit representation) Any octet, except those
//      indicating a line break according to the newline convention
//      of the canonical (standard) form of the data being encoded,
//      may be represented by an "=" followed by a two digit hexadecimal
//      representation of the octet's value. The digits of the hexadecimal
//      alphabet, for this purpose, are "0123456789ABCDEF". Uppercase
//      letters must be used when sending hexadecimal data, though a
//      robust implementation may choose to recognize lowercase letters
//      on receipt. Thus, for example, the value 12 (ASCII form feed)
//      can be represented by "=0C", and the value 61 (ASCII EQUAL SIGN)
//      can be represented by "=3D". Except when the following rules
//      allow an alternative encoding, this rule is mandatory.
//
//      Rule #2: (Literal representation) Octets with decimal values of
//      33 through 60 inclusive, and 62 through 126, inclusive, MAY be
//      represented as the ASCII characters which correspond to those
//      octets (EXCLAMATION POINT through LESS THAN, and GREATER THAN
//      through TILDE, respectively).
//
//      Rule #3: (White Space): Octets with values of 9 and 32 MAY be
//      represented as ASCII TAB (HT) and SPACE characters, respectively,
//      but MUST NOT be so represented at the end of an encoded line. Any
//      TAB (HT) or SPACE characters on an encoded line MUST thus be
//      followed on that line by a printable character. In particular,
//      an "=" at the end of an encoded line, indicating a soft line
//      break (see rule #5) may follow one or more TAB (HT) or SPACE
//      characters. It follows that an octet with value 9 or 32 appearing
//      at the end of an encoded line must be represented according
//      to Rule #1. This rule is necessary because some MTAs (Message
//      Transport Agents, programs which transport messages from one
//      user to another, or perform a part of such transfers) are known
//      to pad lines of text with SPACEs, and others are known to remove
//      "white space" characters from the end of a line. Therefore, when
//      decoding a Quoted-Printable body, any trailing white space on
//      a line must be deleted, as it will necessarily have been added
//      by intermediate transport agents.
//
//      Rule #4 (Line Breaks): A line break in a text body, independent of
//      what its representation is following the canonical representation
//      of the data being encoded, must be represented by a (RFC 822)
//      line break, which is a CRLF sequence, in the Quoted-Printable
//      encoding. Since the canonical representation of types other than
//      text do not generally include the representation of line breaks,
//      no hard line breaks (i.e.  line breaks that are intended to
//      be meaningful and to be displayed to the user) should occur
//      in the quoted-printable encoding of such types. Of course,
//      occurrences of "=0D", "=0A", "0A=0D" and "=0D=0A" will eventually
//      be encountered. In general, however, base64 is preferred over
//      quoted-printable for binary data.
//
//      Note that many implementations may elect to encode the local
//      representation of various content types directly, as described
//      in Appendix G. In particular, this may apply to plain text
//      material on systems that use newline conventions other than
//      CRLF delimiters. Such an implementation is permissible, but the
//      generation of line breaks must be generalized to account for
//      the case where alternate representations of newline sequences
//      are used.
//
//      Rule #5 (Soft Line Breaks): The Quoted-Printable encoding REQUIRES
//      that encoded lines be no more than 76 characters long. If longer
//      lines are to be encoded with the Quoted-Printable encoding,
//      'soft' line breaks must be used. An equal sign as the last
//      character on a encoded line indicates such a non-significant
//      ('soft') line break in the encoded text. Thus if the "raw"
//      form of the line is a single unencoded line that says:
//
//              Now's the time for all folk to come to the aid of their country.
//
//      This can be represented, in the Quoted-Printable encoding, as
//
//              Now's the time =
//              for all folk to come =
//              to the aid of their country.
//
//      This provides a mechanism with which long lines are encoded in
//      such a way as to be restored by the user agent. The 76 character
//      limit does not count the trailing CRLF, but counts all other
//      characters, including any equal signs.
//
//      Since the hyphen character ("-") is represented as itself
//      in the Quoted-Printable encoding, care must be taken, when
//      encapsulating a quoted-printable encoded body in a multipart
//      entity, to ensure that the encapsulation boundary does not
//      appear anywhere in the encoded body. (A good strategy is to
//      choose a boundary that includes a character sequence such as
//      "=_" which can never appear in a quoted- printable body. See
//      the definition of multipart messages later in this document.)
//
//      NOTE: The quoted-printable encoding represents something
//      of a compromise between readability and reliability in
//      transport. Bodies encoded with the quoted-printable encoding
//      will work reliably over most mail gateways, but may not
//      work perfectly over a few gateways, notably those involving
//      translation into EBCDIC. (In theory, an EBCDIC gateway could
//      decode a quoted-printable body and re-encode it using base64,
//      but such gateways do not yet exist.) A higher level of confidence
//      is offered by the base64 Content-Transfer-Encoding. A way to get
//      reasonably reliable transport through EBCDIC gateways is to also
//      quote the ASCII characters
//
//              !"#$@[\]^`{|}~
//
//      according to rule #1. See Appendix B for more information.
//
//      Because quoted-printable data is generally assumed to be line-
//      oriented, it is to be expected that the representation of
//      the breaks between the lines of quoted printable data may
//      be altered in transport, in the same manner that plain text
//      mail has always been altered in Internet mail when passing
//      between systems with differing newline conventions. If such
//      alterations are likely to constitute a corruption of the data,
//      it is probably more sensible to use the base64 encoding rather
//      than the quoted-printable encoding.
//
//      WARNING TO IMPLEMENTORS: If binary data are encoded in quoted-
//      printable, care must be taken to encode CR and LF characters as
//      "=0D" and "=0A", respectively. In particular, a CRLF sequence in
//      binary data should be encoded as "=0D=0A". Otherwise, if CRLF
//      were represented as a hard line break, it might be incorrectly
//      decoded on platforms with different line break conventions.
//
//      For formalists, the syntax of quoted-printable data is described
//      by the following grammar:
//
//      quoted-printable := ([*(ptext / SPACE / TAB) ptext] ["="] CRLF)
//              ; Maximum line length of 76 characters excluding CRLF
//
//      ptext := octet /<any ASCII character except "=", SPACE, or TAB>
//              ; characters not listed as "mail-safe" in Appendix B
//              ; are also not recommended.
//
//      octet := "=" 2(DIGIT / "A" / "B" / "C" / "D" / "E" / "F")
//              ; octet must be used for characters > 127, =, SPACE, or TAB,
//              ; and is recommended for any characters not listed in
//              ; Appendix B as "mail-safe".
//

#include <common/ac/ctype.h>

#include <libaegis/output/filter/quoted_print.h>


static inline char
map_hex(int x)
{
    static char hex_digit[16] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    return hex_digit[x & 15];
}


void
output_filter_quoted_printable::eoln(bool soft)
{
    glyph_t *cp = glyph;
    glyph_t *end = cp + pos;
    for (; cp < end; ++cp)
    {
        unsigned char c = cp->text;
        if (cp->quote_it)
        {
            deeper_fputc('=');
            deeper_fputc(map_hex(c >> 4));
            deeper_fputc(map_hex(c));
        }
        else
            deeper_fputc(c);
    }
    if (soft)
        deeper_fputc('=');
    deeper_fputc('\n');
    pos = 0;
}


output_filter_quoted_printable::output_filter_quoted_printable(
    const output::pointer &a_deeper,
    bool a_intl
) :
    output_filter(a_deeper),
    allow_international_characters(a_intl),
    pos(0)
{
}


output::pointer
output_filter_quoted_printable::create(const output::pointer &a_deeper,
    bool a_intl)
{
    return pointer(new output_filter_quoted_printable(a_deeper, a_intl));
}


void
output_filter_quoted_printable::eoln_partial(void)
{
    //
    // The line is loo long.  We need to back up a few
    // characters.  We must allow one column for the '='
    // soft line break
    // (which is why we wsay >=MAX_LINE_LEN instead of >MAX_LINE_LEN).
    //
    long oldpos = pos;
    long newpos = pos;
    while (newpos > 0 && glyph[newpos - 1].cumulative >= MAX_LINE_LEN)
        --newpos;
    long newpos_max = newpos;

    //
    // It's worth hunting for a white space character, it looks nicer.
    //
    while
    (
        newpos > 0
    &&
        glyph[newpos - 1].text != ' '
    &&
        glyph[newpos - 1].text != '\t'
    )
        --newpos;
    if (newpos == 0)
        newpos = newpos_max;

    //
    // re-write the line length, and emit the partial line.
    //
    pos = newpos;
    eoln(true);

    //
    // Move everything down.
    //
    for (int j = 0; j + newpos < oldpos; ++j)
    {
        glyph_t *gp = glyph + j;
        *gp = glyph[newpos + j];
        gp->cumulative = (j ? gp[-1].cumulative : 0) + gp->width;
    }
    pos = oldpos - newpos;
}


void
output_filter_quoted_printable::eoln_hard(void)
{
    //
    // We are required to quote trailing spaces or tabs.
    //
    if (pos)
    {
        glyph_t         *gp;

        gp = glyph + pos - 1;
        if (gp->text == ' ' || gp->text == '\t')
        {
            gp->quote_it = 1;
            gp->cumulative += 3 - gp->width;
            gp->width = 3;

            //
            // This could make the line longer than the
            // maximum, in which case we need to emit the
            // partial line first.
            //
            if (gp->cumulative > MAX_LINE_LEN)
                eoln_partial();
        }
    }

    //
    // now emit the whole lot.
    //
    eoln(false);
}


output_filter_quoted_printable::~output_filter_quoted_printable()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    while (pos)
        eoln_partial();
}


void
output_filter_quoted_printable::write_inner(const void *p, size_t len)
{
    const unsigned char *data = (const unsigned char *)p;
    while (len > 0)
    {
        unsigned char c = *data++;
        --len;
        if (c == '\n')
        {
            eoln_hard();
            continue;
        }

        glyph_t *gp = glyph + pos;
        gp->text = c;
        gp->width = 1;
        gp->quote_it = false;
        gp->cumulative = 0;
        switch (c)
        {
        case '=':
            gp->width = 3;
            gp->quote_it = true;
            break;

        case '\t':
            {
                int col1 = (pos ? gp[-1].cumulative : 0);
                int col2 = (col1 + 8) & ~7;
                gp->width = col2 - col1;
            }
            break;

        default:
            // C locale
            if
            (
                (allow_international_characters ?  c < ' ' : !isprint(c))
            &&
                !isspace(c)
            )
            {
                gp->width = 3;
                gp->quote_it = true;
            }
            break;
        }
        gp->cumulative = (pos ? gp[-1].cumulative : 0) + gp->width;
        pos++;

        if (gp->cumulative > MAX_LINE_LEN)
            eoln_partial();
    }
}


int
output_filter_quoted_printable::page_width(void)
    const
{
    return MAX_LINE_LEN;
}


void
output_filter_quoted_printable::end_of_line_inner(void)
{
    if (pos)
        eoln_hard();
}


nstring
output_filter_quoted_printable::type_name(void)
    const
{
    return ("quoted_printable " + output_filter::type_name());
}


// vim: set ts=8 sw=4 et :
