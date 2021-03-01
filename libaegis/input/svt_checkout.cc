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
// MANIFEST: implementation of the input_svt_checkout class
//

#pragma implementation "input_svt_checkout"

#include <input/crop.h>
#include <input/file.h>
#include <input/gunzip.h>
#include <input/verify_check.h>
#include <input/svt_checkout.h>
#include <output/bit_bucket.h>
#include <rfc822.h>
#include <sub.h>


input_svt_checkout::~input_svt_checkout()
{
    delete src4_p;
    delete src3_p;
    delete src2_p;
    delete src1_p;
}


input_svt_checkout::input_svt_checkout(const nstring &filename,
	const nstring &version) :
    src1_p(new input_file(filename)),
    src2_p(new input_gunzip(src1_p, false)),
    src3_p(0),
    src4_p(0),
    pos(0)
{
    for (;;)
    {
	//
	// Read an RFC 822 header from the input.
	//
	rfc822 header;
	header.load(*src2_p);

	//
	// If the version matches, extract it to a file.
	//
	long content_length = header.get_long("content-length");
	src3_p = new input_crop(src2_p, false, content_length);
	unsigned long checksum = header.get_ulong("checksum");
	src4_p = new input_verify_checksum(*src3_p, checksum);
	if (version.empty() || version == header.get("version"))
	    return;

	//
	// Throw away the file contents.
	//
	output_bit_bucket bit_bucket;
	bit_bucket << *src4_p;

	delete src4_p;
	src4_p = 0;
	delete src3_p;
	src3_p = 0;

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
    return (src4_p ? src4_p->length() : 0);
}


long
input_svt_checkout::ftell_inner()
{
    return pos;
}


long
input_svt_checkout::read_inner(void *data, size_t nbytes)
{
    if (!src4_p)
	return 0;
    long result = src4_p->read(data, nbytes);
    pos += result;
    return result;
}
