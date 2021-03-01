//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006 Peter Miller;
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
// MANIFEST: functions to deliver output to stdout
//

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/types.h>
#include <sys/stat.h>

#include <common/nstring.h>
#include <common/page.h>
#include <common/trace.h>
#include <libaegis/output/stdout.h>
#include <libaegis/sub.h>


static nstring
standard_output_name(void)
{
    static nstring name;
    if (name.empty())
    {
	sub_context_ty sc;
	name = nstring(sc.subst_intl(i18n("standard output")));
    }
    return name;
}


output_stdout::~output_stdout()
{
    trace(("output_stdout::~output_stdout(this = %08lX)\n{\n", (long)this));
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
    trace(("}\n"));
}


output_stdout::output_stdout() :
    bol(true),
    pos(0)
{
    trace(("output_stdout::output_stdout(this = %08lX)\n", (long)this));
}


string_ty *
output_stdout::filename()
    const
{
    return standard_output_name().get_ref();
}


long
output_stdout::ftell_inner()
    const
{
    trace(("output_stdout::ftell_inner(this = %08lX) returns %ld\n", (long)this,
	pos));
    return pos;
}


void
output_stdout::write_inner(const void *data, size_t len)
{
    trace(("output_stdout::write_inner(this = %08lX, data = %08lX, "
	"len = %ld)\n{\n", (long)this, (long)data, (long)len));
    if (::write(fileno(stdout), data, len) < 0)
    {
	    int errno_old = errno;
	    sub_context_ty sc;
	    sc.errno_setx(errno_old);
	    sc.var_set_string("File_Name", standard_output_name());
	    sc.fatal_intl(i18n("write $filename: $errno"));
	    // NOTREACHED
    }
    if (len > 0)
	bol = (((const char *)data)[len - 1] == '\n');
    pos += len;
    trace(("pos = %ld\n", pos));
    trace(("}\n"));
}


int
output_stdout::page_width()
    const
{
    struct stat	st;
    if (fstat(fileno(stdout), &st) == 0 && S_ISREG(st.st_mode))
	return page_width_get(DEFAULT_PRINTER_WIDTH);
    return page_width_get(-1) - 1;
}


int
output_stdout::page_length()
    const
{
    struct stat	st;
    if (fstat(fileno(stdout), &st) == 0 && S_ISREG(st.st_mode))
	return page_length_get(DEFAULT_PRINTER_LENGTH);
    return page_length_get(-1);
}


void
output_stdout::end_of_line_inner()
{
    trace(("output_stdout::end_of_line_inner(this = %08lX)\n{\n", (long)this));
    if (!bol)
	fputc('\n');
    trace(("pos = %ld\n", pos));
    trace(("}\n"));
}


const char *
output_stdout::type_name()
    const
{
    return standard_output_name().c_str();
}
