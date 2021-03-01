//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the usage class
//

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <progname.h>
#include <usage.h>


void
usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --receive [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --replay [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --missing [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    fprintf(stderr, "       %s --list\n", progname);
    exit(1);
}
