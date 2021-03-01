//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
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
// MANIFEST: implementation of the input_svt_checkout class
//

#include <libaegis/input/bunzip2.h>
#include <libaegis/input/crop.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/verify_check.h>
#include <libaegis/input/svt_checkout.h>
#include <libaegis/output/bit_bucket.h>
#include <libaegis/rfc822.h>
#include <libaegis/sub.h>


input_svt_checkout::~input_svt_checkout()
{
}


static input
unzip_either(input &ifp)
{
    input temp = input_gunzip_open(ifp);
    return input_bunzip2_open(ifp);
}


input_svt_checkout::input_svt_checkout(const nstring &filename,
	const nstring &version) :
    src1_p(new input_file(filename)),
    src2_p(unzip_either(src1_p)),
    pos(0)
{
    for (;;)
    {
	//
	// Read an RFC 822 header from the input.
	//
	rfc822 header;
	header.load(src2_p);

	//
	// If the version matches, extract it to a file.
	//
	long content_length = header.get_long("content-length");
	src3_p = new input_crop(src2_p, content_length);
	unsigned long checksum = header.get_ulong("checksum");
	src4_p = new input_verify_checksum(src3_p, checksum);
	if (version.empty() || version == header.get("version"))
	    return;

	//
	// Throw away the file contents.
	//
	output_bit_bucket bit_bucket;
	bit_bucket << src4_p;

	src4_p.close();
	src3_p.close();

	//
	// If there is no more input, we are done.
	//
	if (src2_p->at_end())
	{
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", filename);
	    sc.var_set_string("Number", version);
	    sc.fatal_intl(i18n("$filename: version $number not found"));
	    // NOTREACHED
	}
    }
}


nstring
input_svt_checkout::name()
{
    return src1_p->name();
}


long
input_svt_checkout::length()
{
    return (src4_p.is_open() ? src4_p->length() : 0);
}


long
input_svt_checkout::ftell_inner()
{
    return pos;
}


long
input_svt_checkout::read_inner(void *data, size_t nbytes)
{
    if (!src4_p.is_open())
	return 0;
    long result = src4_p->read(data, nbytes);
    pos += result;
    return result;
}
