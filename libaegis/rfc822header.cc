//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate headers
//

#include <ac/ctype.h>
#include <ac/string.h>

#include <input/822wrap.h>
#include <input/base64.h>
#include <input/crlf.h>
#include <input/uudecode.h>
#include <mem.h>
#include <rfc822header.h>
#include <stracc.h>
#include <str.h>
#include <symtab.h>
#include <trace.h>


static void
reap(void *p)
{
    string_ty	    *s;

    s = (string_ty *)p;
    str_free(s);
}


static int
has_a_header(input_ty *ifp)
{
    int		    result;
    int		    state;
    size_t	    length_of_garbage = 0;

    trace(("has_a_header(ifp = %08lX)\n{\n", (long)ifp));
    result = 0;
    stracc_t buffer;

    //
    // MH (a mail handler) has a tendancy to add a line of the form
    //	    (Message <folder>:<num>)
    // to the start of the message.  This is particularly irritating
    // when you want to say
    //	    show | aepatch -rec
    //
    // Some other mail readers add a
    //	    From ...
    // to the start of the message.  Toss these, too.
    //
    for (;;)
    {
	int		c;

	c = ifp->getc();
	if (c < 0)
	    break;
	buffer.push_back(c);
	if (c == '\n')
	    break;
    }
    if
    (
	buffer.size() >= 14
    &&
	0 == memcmp("(Message ", buffer.get_data(), 9)
    &&
	buffer[buffer.size() - 2] == ')'
    )
    {
	// it's a garbage MH line, toss it
	length_of_garbage = buffer.size();
    }
    else if (buffer.size() >= 6 && 0 == memcmp("From ", buffer.get_data(), 5))
    {
	// it's a garbage From line, toss it
	length_of_garbage = buffer.size();
    }
    else
    {
	// give the line back
	ifp->unread(buffer.get_data(), buffer.size());
	buffer.clear();
    }

    //
    // Now look for a regular RFC 822 header.
    //
    // State 0: start of line
    // State 1: seen valid name characters
    // State 2: seen colon
    // State 3: finished (see "result")
    //
    state = 0;
    while (state < 3 && buffer.size() < 80)
    {
	int		c;

	c = ifp->getc();
	if (c >= 0)
	    buffer.push_back(c);
	if (c < 0)
	{
	    state = 3;
	    break;
	}
	switch ((unsigned char)c)
	{
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
	    // When this read
	    //	    if (state == 0 || state == 1)
	    //		    state = 1;
	    //	    else
	    //		    state = 3;
	    // gcc 2.96 got this function wrong when you said -O2
	    //
	    if (state == 0)
		state = 1;
	    else if (state != 1)
		state = 3;
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
	case '-':
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
	    length_of_garbage = 0;
	    break;
	}
    }
    ifp->unread
    (
	buffer.get_data() + length_of_garbage,
	buffer.size() - length_of_garbage
    );
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}


static int
is_binary(input_ty *ifp)
{
    int		    result;
    int		    c;

    //
    // We are looking for NULs, because gzipped files have
    // NULs in the header.  This function is only used to fake a
    // Content-Transfer-Encoding line in there case where there is
    // no header to tell us.
    //
    result = 0;
    stracc_t buffer;
    while (buffer.size() < 800)
    {
	c = ifp->getc();
	if (c < 0)
	    break;
	buffer.push_back(c);
	if (c == 0)
	{
	    result = 1;
	    break;
	}
    }
    ifp->unread(buffer.get_data(), buffer.size());
    return result;
}


rfc822_header_ty *
rfc822_header_read(input_ty *ifp)
{
    rfc822_header_ty *hp;

    //
    // Allocate a symbol table to hold everything.
    //
    trace(("rfc822_header_read(ifp = %08lX)\n{\n", (long)ifp));
    hp = (rfc822_header_ty *)mem_alloc(sizeof(rfc822_header_ty));
    hp->stp = new symtab_ty(1);
    hp->stp->set_reap(reap);

    //
    // If there is no 822 header, return an empty symbol table.
    //
    if (!has_a_header(ifp))
    {
	trace(("no header\n"));
	if (is_binary(ifp))
	    ;
	else if (input_base64_recognise(ifp))
	{
	    string_ty	    *name;
	    string_ty	    *value;

	    trace(("looks like base64\n"));
	    name = str_from_c("content-transfer-encoding");
	    value = str_from_c("base64");
	    hp->stp->assign(name, value);
	    str_free(name);
	}
	else if (input_uudecode_recognise(ifp))
	{
	    string_ty	    *name;
	    string_ty	    *value;

	    trace(("looks like uuencode\n"));
	    name = str_from_c("content-transfer-encoding");
	    value = str_from_c("uuencode");
	    hp->stp->assign(name, value);
	    str_free(name);
	}
	trace(("return %08lX\n", (long)hp));
	trace(("}\n"));
	return hp;
    }

    //
    // Do the end-of-line wrapping transparently.
    //
    ifp = new input_crlf(ifp, false);
    ifp = new input_822wrap(ifp, true);
    stracc_t buffer;

    //
    // MH (a mail handler) has a tendancy to add a line if the form
    //	    (Message <folder>:<num>)
    // to the start of the message.  This is particularly irritating
    // when you want to say
    //	    show | aepatch -rec
    //
    for (;;)
    {
	int		c;

	c = ifp->getc();
	if (c < 0)
	    break;
	buffer.push_back(c);
	if (c == '\n')
	    break;
    }
    if
    (
	buffer.size() >= 14
    &&
	0 == memcmp("(Message ", buffer.get_data(), 9)
    &&
	buffer[buffer.size() - 2] == ')'
    )
    {
	// it's a garbage MH line, toss it
    }
    else
    {
	// give the line back
	ifp->unread(buffer.get_data(), buffer.size());
    }
    buffer.clear();

    //
    // Read header lines until we find a blank line.
    //
    for (;;)
    {
	int		c;
	string_ty	*name;
	string_ty	*value;

	//
	// EOF or a blank line stops us.
	//
	c = ifp->getc();
	if (c < 0)
	    break;
	if (c == '\n')
	    break;

	//
	// Read the name.
	// Complain if we don't like the characters.
	//
	buffer.clear();
	for (;;)
	{
	    if (c == ':')
		break;
	    if (c < 0 || (c != '-' && !isalnum((unsigned char)c)))
		ifp->fatal_error("malformed RFC822 header");
	    if (isupper((unsigned char)c))
		c = tolower((unsigned char)c);
	    buffer.push_back(c);
	    c = ifp->getc();
	}
	name = buffer.mkstr();
	if (name->str_length < 1)
	    ifp->fatal_error("malformed RFC822 header (no name)");

	//
	// Skip the white space after the colon.
	//
	for (;;)
	{
	    c = ifp->getc();
	    if (c != ' ' && c != '\t')
		break;
	}

	//
	// Read the value.
	// We aren't so picky this time.
	//
	buffer.clear();
	for (;;)
	{
	    if (c < 0 || c == '\n')
		break;
	    buffer.push_back(c);
	    c = ifp->getc();
	}
	value = buffer.mkstr();

	//
	// assign the value
	//
	hp->stp->assign(name, value);
	str_free(name);
    }
    delete ifp;
    trace(("}\n"));
    trace(("return %08lX\n", (long)hp));
    return hp;
}


void
rfc822_header_delete(rfc822_header_ty *hp)
{
    delete hp->stp;
    hp->stp = 0;
    mem_free(hp);
}


string_ty *
rfc822_header_query(rfc822_header_ty *hp, const char *name)
{
    string_ty	    *Name;
    string_ty	    *Name2;
    string_ty	    *result;

    Name = str_from_c(name);
    Name2 = str_downcase(Name);
    str_free(Name);
    result = (string_ty *)hp->stp->query(Name2);
    str_free(Name2);
    return result;
}
