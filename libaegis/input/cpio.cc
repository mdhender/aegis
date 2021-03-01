//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/nstring/accumulator.h>
#include <common/trace.h>
#include <libaegis/input/cpio.h>
#include <libaegis/input/crop.h>


input_cpio::~input_cpio()
{
}


input_cpio::input_cpio(input &arg) :
    deeper(arg)
{
}


long
input_cpio::read_inner(void *, size_t)
{
    assert(0);
    return -1;
}


long
input_cpio::ftell_inner()
{
    assert(0);
    return 0;
}


nstring
input_cpio::name()
{
    return deeper->name();
}


long
input_cpio::length()
{
    return deeper->length();
}


input
input_cpio::child(nstring &archive_name)
{
    //
    // Return a closed input at end-of-file
    //
    trace(("input_cpio::child(this = %08lX)\n{\n", (long)this));
    if (deeper->peek() < 0)
    {
	trace(("return NULL\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // read the file header
    //
    for (const char *magic = "070701"; *magic; ++magic)
    {
	int c = deeper->getch();
	if (c != *magic)
    	    deeper->fatal_error("cpio: wrong magic number");
    }
    hex8();	// inode
    hex8();	// mode
    hex8();	// uid
    hex8();	// gid
    hex8();	// nlinks
    hex8();	// mtime
    long hlength = hex8();
    trace_long(hlength);
    hex8();	// dev_major
    hex8();	// dev_minor
    hex8();	// rdev_major
    hex8();	// rdev_minor
    long namlen = hex8();
    trace(("namlen = %ld\n", namlen));
    hex8();	// no checksum
    trace(("archive_name = \"%s\"\n", archive_name.c_str()));
    archive_name = get_name(namlen);
    padding();

    //
    // The trailer record tells us when to stop.
    //
    if (archive_name == "TRAILER!!!")
    {
	trace(("NULL\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // Figure out how much of the deeper input is to be cropped out for
    // this child.
    //
    long alength = (hlength + 3) & ~3;
    trace_long(alength);
    input_crop *icp = 0;
    if (alength == hlength)
    {
	icp = new input_crop(deeper, hlength);
    }
    else
    {
	input temp(new input_crop(deeper, alength));
	icp = new input_crop(temp, hlength);
    }

    //
    // Set the name of the child, so we get nice error messages.
    //
    nstring filename =
	nstring::format("%s(%s)", deeper->name().c_str(), archive_name.c_str());
    icp->set_name(filename);

    //
    // Report success.
    //
    trace(("return %08lX\n", (long)icp));
    trace(("}\n"));
    return icp;
}


void
input_cpio::padding()
{
    int n = deeper->ftell();
    n %= 4;
    if (n)
	deeper->skip(4 - n);
}


int
input_cpio::hex_digit(bool &first)
{
    int c = deeper->getch();
    switch (c)
    {
    default:
	fatal_error("cpio: invalid hex digit");
	// NOTREACHED

    case ' ':
	if (first)
    	    return 0;
	fatal_error("cpio: invalid hex number");
	// NOTREACHED

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
	first = false;
	return (c - '0');

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	first = false;
	return (c - 'a' + 10);

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	first = first;
	return (c - 'A' + 10);
    }
}


long
input_cpio::hex8()
{
    long result = 0;
    bool first = true;
    for (int j = 0; j < 8; ++j)
    {
	int c = hex_digit(first);
	result = (result << 4) + c;
    }
    if (first)
	deeper->fatal_error("cpio: invalid hex number");
    return result;
}


nstring
input_cpio::get_name(long namlen)
{
    trace(("input_cpio::get_name(namelen = %ld)\n", namlen));
    static nstring_accumulator name_buffer;

    //
    // make sure out name_buffer is big enough.
    //
    if (namlen < 2)
	deeper->fatal_error("cpio: invalid name length");
    --namlen;

    //
    // Read the filename, checking each character.
    //
    name_buffer.clear();
    for (long j = 0; j < namlen; ++j)
    {
	int c = deeper->getch();
	if (c <= 0)
	    deeper->fatal_error("cpio: short file");
	if (isspace((unsigned char)c))
	    deeper->fatal_error("cpio: invalid name (white space)");
	if (!isprint((unsigned char)c))
	    deeper->fatal_error("cpio: invalid name (unprintable)");
	name_buffer.push_back(c);
    }

    //
    // Must have a NUL on the end.
    //
    if (deeper->getch() != 0)
	deeper->fatal_error("cpio: invalid character");

    //
    // Build the result and return.
    //
trace(("name_buffer = \"%.*s\"\n", (int)name_buffer.size(),
name_buffer.get_data()));
    return name_buffer.mkstr();
}


bool
input_cpio::is_remote()
    const
{
    return deeper->is_remote();
}
