//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006, 2008 Peter Miller
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
#include <common/mem.h>
#include <common/nstring.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/output/memory.h>


output_memory::~output_memory()
{
    trace(("output_memory_destructor(this = %08lX)\n{\n", (long)this));
    trace(("}\n"));
}


output_memory::output_memory()
{
    trace(("output_memory()\n"));
}


output_memory::mpointer
output_memory::create()
{
    return mpointer(new output_memory());
}


nstring
output_memory::filename()
    const
{
    return "memory";
}


long
output_memory::ftell_inner()
    const
{
    return buffer.size();
}


void
output_memory::write_inner(const void *data, size_t len)
{
    trace(("output_memory_write(this = %08lX, data = %08lX, len = %ld)\n{\n",
	(long)this, (long)data, (long)len));
    buffer.push_back((const char *)data, len);
    trace(("}\n"));
}


void
output_memory::end_of_line_inner()
{
    trace(("output_memory_eol(this = %08lX)\n{\n", (long)this));
    if (!buffer.empty() && buffer[buffer.size() - 1] != '\n')
	fputc('\n');
    trace(("}\n"));
}


const char *
output_memory::type_name()
    const
{
    return "memory";
}


void
output_memory::forward(output::pointer deeper)
{
    trace(("output_memory_forward(this = %08lX, deeper = %08lX)\n{\n",
	(long)this, (long)deeper.get()));
    flush();
    if (!buffer.empty())
	deeper->write(buffer.get_data(), buffer.size());
#ifdef DEBUG
    else
	error_raw("%s: %d: nothing to forward", __FILE__, __LINE__);
#endif
    trace(("}\n"));
}


nstring
output_memory::mkstr()
{
    flush();
    return buffer.mkstr();
}
