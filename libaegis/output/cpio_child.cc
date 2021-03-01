//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2006, 2008 Peter Miller
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

#include <common/error.h>
#include <libaegis/output/cpio_child.h>
#include <libaegis/sub.h>


output_cpio_child::~output_cpio_child()
{
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // It is a fatal error if they got the length wrong.
    //
    if (pos != length)
	changed_size();

    //
    // Add padding to brings us up to the correct multiple of bytes.
    //
    padding();

    //
    // DO NOT delete deeper;
    // this is output_cpio::destructor's job.
    //
}


output_cpio_child::output_cpio_child(const output::pointer &a_deeper,
        const nstring &a_name, long a_length, time_t a_mtime) :
    deeper(a_deeper),
    name(a_name),
    length(a_length),
    pos(0),
    bol(true),
    mtime(a_mtime)
{
    assert(length >= 0);
    header();
}


output::pointer
output_cpio_child::create(const output::pointer &a_deeper,
    const nstring &a_name, long a_length, time_t a_mtime)
{
    return pointer(new output_cpio_child(a_deeper, a_name, a_length, a_mtime));
}


void
output_cpio_child::changed_size()
{
    sub_context_ty sc;
    sc.var_set_format
    (
	"File_Name",
	"%s(%s)",
	deeper->filename().c_str(),
	name.c_str()
    );
    sc.fatal_intl(i18n("archive member $filename changed size"));
}


void
output_cpio_child::padding()
{
    long n = deeper->ftell();
    assert(n >= 0);
    for (;;)
    {
	if ((n & 3) == 0)
	    break;
	deeper->fputc('\n');
	++n;
    }
}


void
output_cpio_child::hex8(long n)
{
    deeper->fprintf("%8.8lx", n);
}


void
output_cpio_child::header()
{
    static int ino;
    int inode = ++ino;
    if (name == "TRAILER!!!")
	inode = 0;

    deeper->fputs("070701"); // magic number
    hex8(inode); // inode
    hex8(0100644); // mode
    hex8(0); // uid
    hex8(0); // gid
    hex8(1); // nlinks
    hex8(mtime); // mtime
    hex8(length); // size
    hex8(0); // dev_major
    hex8(0); // dev_minor
    hex8(0); // rdev_major
    hex8(0); // rdev_minor
    hex8(name.length() + 1);
    hex8(0); // no checksum
    deeper->write(name.c_str(), name.length() + 1);
    padding();
}


nstring
output_cpio_child::filename()
    const
{
    return deeper->filename();
}


long
output_cpio_child::ftell_inner()
    const
{
    return pos;
}


void
output_cpio_child::write_inner(const void *data, size_t len)
{
    deeper->write(data, len);
    pos += len;
    if (len > 0)
	bol = (((const char *)data)[len - 1] == '\n');
}


void
output_cpio_child::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_cpio_child::type_name()
    const
{
    return "cpio child";
}
