//
// aegis - project change supervisor
// Copyright (C) 1999, 2003-2006, 2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

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
        string_ty *tmp = sc.subst_intl(i18n("standard output"));
        name = nstring(tmp);
        str_free(tmp);
    }
    return name;
}


output_stdout::~output_stdout()
{
    trace(("output_stdout::~output_stdout(this = %p)\n{\n", this));
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
    trace(("output_stdout::output_stdout(this = %p)\n", this));
}


output::pointer
output_stdout::create(void)
{
    return pointer(new output_stdout());
}


nstring
output_stdout::filename(void)
    const
{
    return standard_output_name();
}


long
output_stdout::ftell_inner(void)
    const
{
    trace(("output_stdout::ftell_inner(this = %p) returns %ld\n", this,
        pos));
    return pos;
}


void
output_stdout::write_inner(const void *data, size_t len)
{
    trace(("output_stdout::write_inner(this = %p, data = %p, "
        "len = %ld)\n{\n", this, data, (long)len));
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
output_stdout::page_width(void)
    const
{
    struct stat st;
    if (fstat(fileno(stdout), &st) == 0 && S_ISREG(st.st_mode))
        return page_width_get(DEFAULT_PRINTER_WIDTH);
    return page_width_get(-1) - 1;
}


int
output_stdout::page_length(void)
    const
{
    struct stat st;
    if (fstat(fileno(stdout), &st) == 0 && S_ISREG(st.st_mode))
        return page_length_get(DEFAULT_PRINTER_LENGTH);
    return page_length_get(-1);
}


void
output_stdout::end_of_line_inner(void)
{
    trace(("output_stdout::end_of_line_inner(this = %p)\n{\n", this));
    if (!bol)
        fputc('\n');
    trace(("pos = %ld\n", pos));
    trace(("}\n"));
}


nstring
output_stdout::type_name(void)
    const
{
    return standard_output_name();
}


// vim: set ts=8 sw=4 et :
