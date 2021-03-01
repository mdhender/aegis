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

#include <common/progname.h>
#include <common/quit.h>
#include <aeannotate/usage.h>


void
usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --receive [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    quit(1);
}
