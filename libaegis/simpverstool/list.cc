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
// MANIFEST: implementation of the simpverstool_list class
//

#include <ac/stdio.h>

#include <input/file.h>
#include <input/gunzip.h>
#include <input/crop.h>
#include <output/bit_bucket.h>
#include <rfc822/functor.h>
#include <simpverstool.h>


bool
simple_version_tool::list(rfc822_functor &arg)
{
    input_file in_u(history_file_name, false, true);
    if (in_u.length() == 0)
	return true;
    input_gunzip in(&in_u, false);
    for (;;)
    {
	//
	// Read an RFC 822 header from the input.
	//
	rfc822 header;
	header.load(in);

	//
        // Pass the header to the given functor to do whatever it is
        // they want done with the meta-data.
	// Stop if they don't want to keep going.
	//
	if (!arg(header))
	    return false;

	//
	// Throw away the file contents.
        //
        // (Because we read the whole file, and the whole-file checksum
        // will be verified at the end of input, we don't need to check
        // the checksum for individual versions.)
	//
	long content_length = header.get_long("content-length");
	output_bit_bucket bit_bucket;
	input_crop in2(&in, false, content_length);
	bit_bucket << in2;

	//
	// If there is no more input, we are done.
	//
	if (in.at_end())
	    break;
    }
    return true;
}
