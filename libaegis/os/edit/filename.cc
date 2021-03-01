//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate filenames
//

#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <os.h>
#include <progname.h>


string_ty *
os_edit_filename(int at_home)
{
    static int      num;
    string_ty       *buffer;
    string_ty       *result;
    const char      *dir;
    int             name_max;

    if (at_home)
    {
	dir = getenv("HOME");
	if (!dir || dir[0] != '/')
	    dir = "/tmp";
    }
    else
    {
	dir = os_tmpdir()->str_text;
    }
    name_max = 14;
    buffer = str_format("-%d-%d", getpid(), ++num);
    result =
	str_format
	(
	    "%s/%.*s%s",
	    dir,
	    (int)(name_max - buffer->str_length),
	    progname_get(),
	    buffer->str_text
	);
    str_free(buffer);
    return result;
}
