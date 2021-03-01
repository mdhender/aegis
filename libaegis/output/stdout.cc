//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2005 Peter Miller;
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

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/unistd.h>
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <nstring.h>
#include <output/stdout.h>
#include <page.h>
#include <sub.h>


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
    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();
}


output_stdout::output_stdout() :
    bol(true)
{
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
    return lseek(fileno(stdout), 0L, SEEK_CUR);
}


void
output_stdout::write_inner(const void *data, size_t len)
{
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
    if (!bol)
	fputc('\n');
}


const char *
output_stdout::type_name()
    const
{
    return standard_output_name().c_str();
}
