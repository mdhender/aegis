//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/time.h>

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/nstring/accumulator.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/boolean.h>
#include <libaegis/input/822wrap.h>
#include <libaegis/input/base64.h>
#include <libaegis/input/crlf.h>
#include <libaegis/input/file.h>
#include <libaegis/input/uudecode.h>
#include <libaegis/output/file.h>
#include <libaegis/rfc822.h>
#include <libaegis/sub.h>


rfc822::~rfc822()
{
}


rfc822::rfc822()
{
    database.set_reaper();
}


rfc822::rfc822(const rfc822 &arg)
{
    database.set_reaper();
    database = arg.database;
}


rfc822 &
rfc822::operator=(const rfc822 &arg)
{
    if (this != &arg)
	database = arg.database;
    return *this;
}


nstring
rfc822::sanitize_name(const nstring &name)
{
    nstring_accumulator acc;
    const char *cp = name.c_str();
    for (;;)
    {
	unsigned char c = *cp++;
	switch (c)
	{
	case 0:
	    while (acc.back() == '-')
		acc.pop_back();
	    if (acc.empty())
		acc.push_back("empty");
	    return acc.mkstr();

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '_':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z':
	    acc.push_back(c);
	    break;

	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	case 'Y': case 'Z':
	    acc.push_back(tolower(c));
	    break;

	default:
	    if (!acc.empty() && acc.back() != '-')
		acc.push_back('-');
	    break;
	}
    }
}


void
rfc822::set(const nstring &name, const nstring &value)
{
    //
    // Make sure the name is RFC 822 compliant.
    //
    database.assign(sanitize_name(name), value);
}


void
rfc822::set_minimalist(const nstring &name, const nstring &value)
{
    nstring saname = sanitize_name(name);
    if (value.empty())
	database.remove(saname);
    else
	database.assign(saname, value);
}


void
rfc822::set(const nstring &name, bool value)
{
    set(name, bool_to_string(value));
}


void
rfc822::set(const nstring &name, long value)
{
    set(name, nstring::format("%ld", value));
}


void
rfc822::set(const nstring &name, unsigned long value)
{
    set(name, nstring::format("%lu", value));
}


const nstring &
rfc822::get(const nstring &name)
{
    //
    // Look in the database for the named item.  The names are all
    // case-insensitive and are stored in lower case.
    //
    nstring lc_name(sanitize_name(name));
    nstring *value_p = database.query(lc_name);
    if (!value_p)
    {
	//
	// Default it to the empty string.
	//
	database.assign(lc_name, nstring());
	value_p = database.query(lc_name);
	assert(value_p);
    }

    //
    // Return the value found.
    //
    return *value_p;
}


nstring
rfc822::get(const nstring &name)
    const
{
    //
    // Look in the database for the named item.  The names are all
    // case-insensitive and are stored in lower case.
    //
    nstring lc_name(sanitize_name(name));
    nstring *value_p = database.query(lc_name);
    return (value_p ? *value_p : "");
}


long
rfc822::get_long(const nstring &name)
{
    const nstring &value = get(name);
    return strtol(value.c_str(), 0, 10);
}


unsigned long
rfc822::get_ulong(const nstring &name)
{
    const nstring &value = get(name);
    return strtoul(value.c_str(), 0, 10);
}


static bool
has_a_header(input &ifp)
{
    trace(("has_a_header()\n{\n"));
    bool result = false;
    nstring_accumulator buffer;

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
	int c = ifp->getch();
	if (c < 0)
	    break;
	buffer.push_back(c);
	if (c == '\n')
	    break;
    }
    int length_of_garbage = 0;
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
    int state = 0;
    while (state < 3 && buffer.size() < 80)
    {
	int c = ifp->getch();
	if (c >= 0)
	    buffer.push_back(c);
	if (c < 0)
	{
	    state = 3;
	    break;
	}
	switch ((unsigned char)c)
	{
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	case 'v': case 'w': case 'x': case 'y': case 'z':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	case 'V': case 'W': case 'X': case 'Y': case 'Z':
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

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
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
		result = true;
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


static bool
is_binary(input &ifp)
{
    //
    // We are looking for NULs, because gzipped files have
    // NULs in the header.  This function is only used to fake a
    // Content-Transfer-Encoding line in there case where there is
    // no header to tell us.
    //
    bool result = false;
    nstring_accumulator buffer;
    while (buffer.size() < 800)
    {
	int c = ifp->getch();
	if (c < 0)
	    break;
	buffer.push_back(c);
	if (c == 0)
	{
	    result = true;
	    break;
	}
    }
    ifp->unread(buffer.get_data(), buffer.size());
    return result;
}


void
rfc822::load(input &source, bool maybe_not)
{
    trace(("rfc822::load()\n{\n"));

    //
    // If there is no 822 header, return an empty symbol table.
    //
    // Note: when has_a_header returns, no header lines have been
    // consumed, EXCEPT that garbage from MH and some mail readers has
    // been removed.
    //
    if (maybe_not && !has_a_header(source))
    {
	trace(("no header\n"));
	if (is_binary(source))
	{
	    trace(("looks like raw binary\n"));
	}
	else if (input_base64::recognise(source))
	{
	    trace(("looks like base64\n"));
	    set(nstring("content-transfer-encoding"), nstring("base64"));
	}
	else if (input_uudecode::candidate(source))
	{
	    trace(("looks like uuencode\n"));
	    set(nstring("content-transfer-encoding"), nstring("uuencode"));
	}
	trace(("}\n"));
	return;
    }

    //
    // Do the end-of-line wrapping transparently.
    //
    trace(("we expect to see a header\n"));
    input ifp2 = new input_crlf(source);
    input ifp = new input_822wrap(ifp2);

    //
    // We are looking for RFC822 style header lines.
    // When we see a blank line, we stop.
    //
    nstring_accumulator name;
    nstring_accumulator value;
    for (;;)
    {
	int c = ifp->getch();
	if (c < 0 || c == '\n')
	    break;
	name.clear();
	for (;;)
	{
	    switch ((unsigned char)c)
	    {
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
	    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
	    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
	    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
	    case 'y': case 'z':
	    case '0': case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	    case '_': case '-':
		name.push_back(c);
		c = ifp->getch();
		if (c < 0)
		    goto malformed;
		continue;

	    default:
		break;
	    }
	    break;
	}
	trace(("name = \"%s\"\n", name.mkstr().c_str()));
	if (name.empty() || c != ':')
	{
	    malformed:
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", ifp->name());
	    sc.fatal_intl(i18n("$filename: malformed RFC 822 header line"));
	    // NOTREACHED
	}

	//
	// Skip white space between the colon and the start of the
	// value.  (There is no way to have a value which consists
	// entirely of white space.)
	//
	for (;;)
	{
	    c = ifp->getch();
	    if  (c < 0 || c == '\n' || !isspace((unsigned char)c))
		break;
	}

	//
	// Read the value.
	//
	value.clear();
	for (;;)
	{
	    if (c < 0)
		break;
	    if (c == '\n')
		break;
	    value.push_back(c);
	    c = ifp->getch();
	}

	//
	// Remove trailing white space.
	//
	while (isspace((unsigned char)value.back()))
	    value.pop_back();
	trace(("value = \"%s\"\n", value.mkstr().c_str()));

	//
        // Stash the name-value pair in the database.  This means later
        // lines silently over-ride earlier lines.  We aren't expecting
        // duplicates, but some uses of this class can take advantage of
        // this behaviour.
	//
	set(name.mkstr(), value.mkstr());
    }
    trace(("}\n"));
}


void
rfc822::load_from_file(const nstring &filename)
{
    input in = new input_file(filename, false, true);
    load(in);
}


void
rfc822::store(output::pointer dest)
{
    nstring_list names;
    database.keys(names);
    names.sort();
    for (size_t j = 0; j < names.size(); ++j)
    {
	const nstring &name = names[j];
	dest << name.capitalize() << ": ";

	//
        // We have to make sure that the value is written in an RFC822
        // compliant format.  That means we have to be careful of
        // newlines embedded in the value and make sure they are always
        // followed by a space or a tab.
	//
	const nstring &value = get(name);
	const char *cp = value.c_str();
	const char *end = cp + value.size();
	while (end > cp && isspace((unsigned char)end[-1]))
	    --end;
	while (cp < end)
	{
	    unsigned char c = *cp++;
	    dest->fputc(c);
	    if (c == '\n' && *cp != ' ' && *cp != '\t')
		dest->fputc(' ');
	}
	dest->fputc('\n');
    }
    dest->fputc('\n');
}


void
rfc822::store_to_file(const nstring &filename)
{
    output::pointer os = output_file::text_open(filename);
    store(os);
}


nstring
rfc822::date()
{
    time_t now;
    time(&now);
    struct tm *now_tm = localtime(&now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%a, %e %b %Y %H:%M:%S %z", now_tm);
    return buffer;
}


bool
rfc822::is_set(const nstring &name)
    const
{
    //
    // Look in the database for the named item.  The names are all
    // case-insensitive and are stored in lower case.
    //
    nstring lc_name(sanitize_name(name));
    nstring *value_p = database.query(lc_name);
    return (value_p != 0);
}


void
rfc822::clear()
{
    database.clear();
}
