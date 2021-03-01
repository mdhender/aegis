//
// aegis - project change supervisor
// Copyright (C) 2008, 2009, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/wrap.h>

#include <aemakegen/target/debian.h>


void
target_debian::gen_copyright()
{
    nstring text =
        get_cp()->pconf_attributes_find("aemakegen:debian:copyright");
    if (text == "false")
        return;

    nstring who(get_cp()->pconf_copyright_owner_get());
    nstring when(get_cp()->copyright_years_get_string());

    // write debian/copyright
    os_become_orig();
    output::pointer fp = output_file::open("debian/copyright");
    os_become_undo();
    fp->fputs("Copyright (C) ");
    fp->fputs(when);
    fp->fputc(' ');
    fp->fputs(who);
    fp->fputc('\n');

    if (!text.empty())
    {
        fp->fputc('\n');
        output::pointer wop = output_wrap_open(fp, 80);
        wop->fputs(text);
        wop->end_of_line();
    }
}


// vim: set ts=8 sw=4 et :
