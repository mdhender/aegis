//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the rfc822 class
//

#pragma implementation "rfc822"

#include <ac/ctype.h>
#include <ac/stdlib.h>
#include <ac/stdio.h> // for snprintf
#include <ac/time.h>

#include <boolean.h>
#include <error.h> // for assert
#include <input/file.h>
#include <output/file.h>
#include <quit.h>
#include <nstring/accumulator.h>
#include <rfc822.h>
#include <sub.h>


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
    char buf[20];
    snprintf(buf, sizeof(buf), "%ld", value);
    set(name, nstring(buf));
}


void
rfc822::set(const nstring &name, unsigned long value)
{
    char buf[20];
    snprintf(buf, sizeof(buf), "%lu", value);
    set(name, nstring(buf));
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


void
rfc822::load(input_ty &source)
{
    //
    // We are looking for RFC822 style header lines.
    // When we see a blank line, we stop.
    //
    nstring_accumulator name;
    nstring_accumulator value;
    for (;;)
    {
	int c = source.getc();
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
		c = source.getc();
		if (c < 0)
		    goto malformed;
		continue;

	    default:
		break;
	    }
	    break;
	}
	if (name.empty() || c != ':')
	{
	    malformed:
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", source.name());
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
	    c = source.getc();
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
	    {
		c = source.getc();
		if (c < 0)
		    break;
		if (c == '\n' || !isspace((unsigned char)c))
		{
		    source.ungetc(c);
		    break;
		}
		value.push_back('\n');
	    }
	    value.push_back(c);
	    c = source.getc();
	}

	//
	// Remove trailing white space.
	//
	while (isspace((unsigned char)value.back()))
	    value.pop_back();

	//
        // Stash the name-value pair in the database.  This means later
        // lines silently over-ride earlier lines.  We aren't expecting
        // duplicates, but some uses of this class can take advantage of
        // this behaviour.
	//
	set(name.mkstr(), value.mkstr());
    }
}


void
rfc822::load_from_file(const nstring &filename)
{
    input_file in(filename, false, true);
    load(in);
}


void
rfc822::store(output_ty &dest)
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
        // newlines embedded in the value embedded in the value and make
        // sure they are always followed by a space or a tab.
	//
	const nstring &value = get(name);
	const char *cp = value.c_str();
	const char *end = cp + value.size();
	while (end > cp && isspace((unsigned char)end[-1]))
	    --end;
	while (cp < end)
	{
	    unsigned char c = *cp++;
	    dest << c;
	    if (c == '\n' && *cp != ' ' && *cp != '\t')
		dest << ' ';
	}
	dest << '\n';
    }
    dest << '\n';
}


void
rfc822::store_to_file(const nstring &filename)
{
    output_file os(filename);
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
