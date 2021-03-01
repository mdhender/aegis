//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <aetar/header.h>
#include <libaegis/input/crop.h>
#include <aetar/input/tar.h>
#include <common/trace.h>


input_tar::~input_tar()
{
    trace(("~input_tar()\n"));
}


input_tar::input_tar(input &arg) :
    deeper(arg)
{
    trace(("input_tar()\n"));
}


long
input_tar::read_inner(void *, size_t)
{
    trace(("input_tar::read_inner()\n"));
    assert(0);
    return -1;
}


long
input_tar::ftell_inner()
{
    trace(("input_tar::ftell_inner()\n"));
    assert(0);
    return 0;
}


nstring
input_tar::name()
{
    return deeper->name();
}


long
input_tar::length()
{
    trace(("input_tar::length()\n"));
    return deeper->length();
}


static bool
all_zero(const char *buf, size_t len)
{
    while (len-- > 0)
    {
	if (*buf++)
	    return false;
    }
    return true;
}


input
input_tar::child(nstring &archive_name, bool &executable)
{
    //
    // Wade through the garbage until we find something interesting.
    //
    trace(("input_tar::child()\n{\n"));
    nstring longname;
    for (;;)
    {
	//
	// read the file header
	//
	char header[TBLOCK];
	if
	(
	    !deeper->read_strict(header, sizeof(header))
	||
	    all_zero(header, sizeof(header))
	)
	{
	    trace(("return NULL\n"));
	    trace(("}\n"));
	    return 0;
	}
	header_ty *hp = (header_ty *)header;

        //
        // Get file mode
        //
        int mode = header_mode_get(hp);
        executable = ((mode & 0111) != 0);

	//
	// Verify checksum.
	//
	int hchksum = header_checksum_get(hp);
	if (hchksum < 0)
	    deeper->fatal_error("tar: corrupted checksum field");
	int cs2 = header_checksum_calculate(hp);
	if (hchksum != cs2)
	{
	    header_dump(hp);
	    nstring s =
		nstring::format
		(
		    "tar: checksum does not match "
			"(calculated 0%o, file has 0%o)",
		    cs2,
		    hchksum
		);
	    deeper->fatal_error(s.c_str());
	}

	//
	// The essential information we want from the header is the
	// file's name and the file's size.  All that other guff is ignored.
	//
	nstring aname;
	if (!longname.empty())
	{
	    aname = longname;
	    longname.clear();
	}
	else
	{
	    aname = header_name_get(hp);
	    if (aname.empty())
		deeper->fatal_error("tar: corrupted name field");
	}
	long hsize = header_size_get(hp);
	if (hsize < 0)
	    deeper->fatal_error("tar: corrupted size field");

	//
	// Work out what to do with it.
	//
	switch (header_linkflag_get(hp))
	{
	case LF_OLDNORMAL:
	case LF_NORMAL:
	    trace(("normal\n"));
	    if (aname[aname.size() - 1] == '/')
	    {
		//
		// Throw directories away.  Aegis only likes real files.
		//
		continue;
	    }
	    break;

	case LF_CONTIG:
	    trace(("contig\n"));
	    break;

	case LF_LINK:
	case LF_SYMLINK:
	case LF_CHR:
	case LF_BLK:
	case LF_DIR:
	case LF_FIFO:
	    //
	    // Throw these away.  Aegis only likes real files.
	    //
	    trace(("throw this one away\n"));
	    continue;

	case LF_LONGNAME:
	    //
	    // The next real file gets this_thing as its name.
	    //
	    trace(("longname\n"));
	    longname = read_data_as_string(hsize);
	    continue;

	case LF_LONGLINK:
	    //
	    // The next file gets this_thing as its link name.
	    // (But we toss links, so toss the data).
	    //
	    trace(("longlink\n"));
	    deeper->skip(hsize);
	    padding();
	    continue;

	default:
	    {
		trace(("mystery file\n"));
		nstring s =
		    nstring::format
		    (
			"tar: file type \"%c\" unknown",
			header_linkflag_get(hp)
		    );
		deeper->fatal_error(s.c_str());
	    }
	    continue;
	}

	//
	// Create a new input instance.
	// the child will read everything.
	//
	trace(("real file\n"));
	archive_name = aname;
	long asize = (hsize + TBLOCK - 1) & ~(TBLOCK - 1);
	input_crop *sub = 0;
	if (asize == hsize)
	{
	    sub = new input_crop(deeper, hsize);
	}
	else
	{
	    input temp = new input_crop(deeper, asize);
	    sub = new input_crop(temp, hsize);
	}
	sub->set_name
       	(
	    nstring::format("%s(%s)", deeper->name().c_str(), aname.c_str())
       	);
	trace(("return %08lX\n", (long)sub));
	trace(("}\n"));
	return sub;
    }
}


void
input_tar::padding()
{
    trace(("input_tar::padding()\n"));
    int n = deeper->ftell();
    n %= TBLOCK;
    if (n)
	deeper->skip(TBLOCK - n);
    trace(("}\n"));
}


nstring
input_tar::read_data_as_string(size_t size)
{
    trace(("input_tar::read_data_as_string(size = %ld)\n{\n", (long)size));
    static char	*strbuf;
    static size_t maximum;

    if (size > maximum)
    {
	for (;;)
	{
	    maximum = maximum * 2 + 32;
	    if (maximum >= size)
		break;
	}
	delete strbuf;
	strbuf = new char [maximum];
    }
    deeper->read_strictest(strbuf, size);
    while (size > 0 && strbuf[size - 1] == 0)
	--size;
    padding();
    nstring result = nstring(strbuf, size);
    trace(("return \"%s\";\n", result.c_str()));
    trace(("}\n"));
    return result;
}
