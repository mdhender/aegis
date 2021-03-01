//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
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
// MANIFEST: functions to manipulate memorys
//

#include <common/ac/string.h>

#include <common/error.h>
#include <libaegis/output/memory.h>
#include <common/str.h>
#include <common/trace.h>


output_memory_ty::~output_memory_ty()
{
    trace(("output_memory_destructor(this = %08lX)\n{\n", (long)this));
    flush();
    delete [] buffer;
    buffer = 0;
    trace(("}\n"));
}


output_memory_ty::output_memory_ty() :
    buffer(0),
    size(0),
    maximum(0)
{
    trace(("output_memory_ty()\n"));
}


string_ty *
output_memory_ty::filename()
    const
{
    static string_ty *s;
    if (!s)
	s = str_from_c("memory");
    return s;
}


long
output_memory_ty::ftell_inner()
    const
{
    return size;
}


void
output_memory_ty::write_inner(const void *data, size_t len)
{
    trace(("output_memory_write(this = %08lX, data = %08lX, len = %ld)\n{\n",
	(long)this, (long)data, (long)len));
    if (size + len > maximum)
    {
	size_t new_maximum = 2 * maximum + 32;
	while (size + len > new_maximum)
	    new_maximum = 2 * new_maximum + 32;
	unsigned char *new_buffer = new unsigned char [new_maximum];
	if (size)
	    memcpy(new_buffer, buffer, size);
	delete [] buffer;
	buffer = new_buffer;
	maximum = new_maximum;
    }
    memcpy(buffer + size, data, len);
    size += len;
    trace(("}\n"));
}


void
output_memory_ty::end_of_line_inner()
{
    trace(("output_memory_eol(this = %08lX)\n{\n", (long)this));
    if (size && buffer[size - 1] != '\n')
	fputc('\n');
    trace(("}\n"));
}


const char *
output_memory_ty::type_name()
    const
{
    return "memory";
}


void
output_memory_ty::forward(output_ty *deeper)
{
    trace(("output_memory_forward(this = %08lX, deeper = %08lX)\n{\n",
	(long)this, (long)deeper));
    flush();
    if (size)
	deeper->write(buffer, size);
#ifdef DEBUG
    else
	error_raw("%s: %d: nothing to forward", __FILE__, __LINE__);
#endif
    trace(("}\n"));
}
