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

#include <common/ac/stdio.h>

#include <common/mem.h>
#include <libaegis/input/bunzip2.h>
#include <libaegis/input/file.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/crop.h>
#include <libaegis/output/bit_bucket.h>
#include <libaegis/rfc822/functor.h>
#include <libaegis/simpverstool.h>


bool
simple_version_tool::list(rfc822_functor &arg)
{
    input in_u = new input_file(history_file_name, false, true);
    if (in_u->length() == 0)
	return true;
    input temp = input_gunzip_open(in_u);
    input in = input_bunzip2_open(temp);
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
	output::pointer bb = output_bit_bucket::create();
	input in2 = new input_crop(in, content_length);
	bb << in2;
	in2.close();

	//
	// If there is no more input, we are done.
	//
	if (in->at_end())
	    break;
    }
    return true;
}
