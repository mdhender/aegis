//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate tar_childs
//

#include <ac/string.h>

#include <error.h>
#include <header.h>
#include <now.h>
#include <output/tar_child.h>
#include <sub.h>


output_tar_child_ty::~output_tar_child_ty()
{
    //
    // Make sure all buffered data has ben passed to our write_inner
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
}


output_tar_child_ty::output_tar_child_ty(output_ty *arg1, const nstring &arg2,
	long arg3, bool arg4) :
    deeper(arg1),
    name(arg2),
    length(arg3),
    executable(arg4),
    pos(0),
    bol(true)
{
    // assert(length >= 0);
    header();
}


void
output_tar_child_ty::changed_size()
{
    sub_context_ty sc;
    sc.var_set_format
    (
	"File_Name",
	"%s(%s)",
	deeper->filename()->str_text,
	name.c_str()
    );
    sc.fatal_intl(i18n("archive member $filename changed size"));
}


void
output_tar_child_ty::padding()
{
    int n = deeper->ftell() % TBLOCK;
    if (n == 0)
	return;
    while (n++ < TBLOCK)
	deeper->fputc('\n');
}


void
output_tar_child_ty::header()
{
    char tblock[TBLOCK];
    header_ty *hp = (header_ty *)tblock;
    nstring root("root");

    //
    // Long names get special treatment.
    //
    if (name.length() >= sizeof(hp->name))
    {
	memset(tblock, 0, sizeof(tblock));
	strlcpy(hp->name, "././@LongLink", sizeof(hp->name));
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
	// This write, and the leangth in the header, include the
	// terminating NUL on the end of the file name.
       	//
	deeper->write(name.c_str(), name.length() + 1);
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
}


string_ty *
output_tar_child_ty::filename()
    const
{
    return deeper->filename();
}


long
output_tar_child_ty::ftell_inner()
    const
{
    return pos;
}


void
output_tar_child_ty::write_inner(const void *data, size_t len)
{
    deeper->write(data, len);
    pos += len;
    if (len > 0)
	bol = (((const char *)data)[len - 1] == '\n');
}


void
output_tar_child_ty::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_tar_child_ty::type_name()
    const
{
    return "tar child";
}
