//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <common/error.h>
#include <common/now.h>
#include <common/trace.h>
#include <libaegis/sub.h>

#include <aetar/header.h>
#include <aetar/output/tar_child.h>


output_tar_child::~output_tar_child()
{
    trace(("output_tar_child::~output_tar_child(this = %08lX)\n{\n",
	(long)this));
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // It is a bug if the size changed.
    //
    if (pos != length)
    {
	changed_size();
    }

    //
    // Pad the data so that we are a multiple of 512 bytes.
    //
    padding();

    //
    // DO NOT delete deeper;
    // this is output_tar::destructor's job.
    //
    trace(("}\n"));
}


output_tar_child::output_tar_child(const output::pointer &arg1,
        const nstring &arg2, long arg3, bool arg4) :
    deeper(arg1),
    name(arg2),
    length(arg3),
    executable(arg4),
    pos(0),
    bol(true)
{
    trace(("output_tar_child::output_tar_child(this = %08lX)\n{\n",
	(long)this));
    // assert(length >= 0);
    trace(("deeper pos = %ld\n", deeper->ftell()));
    header();
    trace(("}\n"));
}

output::pointer
output_tar_child::create(const output::pointer &a_deeper, const nstring &a_name,
    long a_length, bool a_exec)
{
    return pointer(new output_tar_child(a_deeper, a_name, a_length, a_exec));
}


void
output_tar_child::changed_size()
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
output_tar_child::padding()
{
    trace(("output_tar_child::padding(this = %08lX)\n{\n", (long)this));
    int n = deeper->ftell();
    trace(("n = %d\n", n));
    for (;;)
    {
	if ((n & (TBLOCK - 1)) == 0)
	{
	    trace(("deeper pos = %ld\n", deeper->ftell()));
	    trace(("}\n"));
	    return;
	}
	deeper->fputc('\n');
	++n;
    }
}


void
output_tar_child::header()
{
    trace(("output_tar_child::header(this = %08lX)\n{\n", (long)this));
    char tblock[TBLOCK];
    header_ty *hp = (header_ty *)tblock;
    nstring root("root");

    //
    // Long names get special treatment.
    //
    if (name.length() >= sizeof(hp->name))
    {
	memset(tblock, 0, sizeof(tblock));
	strendcpy(hp->name, "././@LongLink", hp->name + sizeof(hp->name));
	header_size_set(hp, name.length() + 1);
	header_mode_set(hp, 0);
	header_uid_set(hp, 0);
	header_uname_set(hp, root);
	header_gid_set(hp, 0);
	header_gname_set(hp, root);
	header_mtime_set(hp, 0);
	header_linkflag_set(hp, LF_LONGNAME);
	header_checksum_set(hp, header_checksum_calculate(hp));
	deeper->write(tblock, TBLOCK);

	//
	// This write, and the length in the header, include the
	// terminating NUL on the end of the file name.
       	//
	deeper->write(name.c_str(), name.length() + 1);
	trace(("deeper pos = %ld\n", deeper->ftell()));
	padding();
    }

    memset(tblock, 0, sizeof(tblock));
    header_name_set(hp, name);
    header_size_set(hp, length);
    header_mode_set(hp, 0100644 | (executable ? 0111 : 0));
    header_uid_set(hp, 0);
    header_uname_set(hp, root);
    header_gid_set(hp, 0);
    header_gname_set(hp, root);
    header_mtime_set(hp, now());
    header_linkflag_set(hp, LF_NORMAL);
    header_checksum_set(hp, header_checksum_calculate(hp));
    deeper->write(tblock, TBLOCK);
    trace(("deeper pos = %ld\n", deeper->ftell()));
    trace(("}\n"));
}


nstring
output_tar_child::filename()
    const
{
    return deeper->filename();
}


long
output_tar_child::ftell_inner()
    const
{
    return pos;
}


void
output_tar_child::write_inner(const void *data, size_t len)
{
    trace(("output_tar_child::write_inner(this = %08lX, data = %08lX, "
	"len = %ld)\n{\n", (long)this, (long)data, (long)len));
    deeper->write(data, len);
    pos += len;
    if (len > 0)
	bol = (((const char *)data)[len - 1] == '\n');
    trace(("deeper pos = %ld\n", deeper->ftell()));
    trace(("}\n"));
}


void
output_tar_child::end_of_line_inner()
{
    trace(("output_tar_child::end_of_line_inner(this = %08lX)\n{\n",
	(long)this));
    if (!bol)
	fputc('\n');
    trace(("deeper pos = %ld\n", deeper->ftell()));
    trace(("}\n"));
}


const char *
output_tar_child::type_name()
    const
{
    return "tar child";
}
