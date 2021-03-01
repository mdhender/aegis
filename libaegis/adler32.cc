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

#include <common/ac/zlib.h>

#include <libaegis/adler32.h>
#include <libaegis/input/file.h>


unsigned long
calculate_adler32(const nstring &filename)
{
    input_file in(filename);
    uLong check_sum = adler32(0L, Z_NULL, 0);
    for (;;)
    {
	Bytef buf[1024];
	size_t len = in.read(buf, sizeof(buf));
	if (!len)
	    break;
	check_sum = adler32(check_sum, buf, len);
    }
    return check_sum;
}
