//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to copy inputs to outputs
//

#include <input.h>
#include <output.h>


void
input_to_output(input_ty *ifp, output_ty *ofp)
{
    unsigned char   buffer[4096];
    size_t          n;

    for (;;)
    {
	n = input_read(ifp, buffer, sizeof(buffer));
	if (!n)
    	    break;
	output_write(ofp, buffer, n);
    }
}
