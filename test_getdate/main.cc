//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006 Peter Miller;
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
// MANIFEST: functions to manipulate mains
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/arglex.h>
#include <common/error.h>
#include <common/gettime.h>
#include <common/progname.h>


static void
usage(void)
{
    const char	*progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"Usage: %s textual date\n",
	progname
    );
    exit(1);
}


int
main(int argc, char **argv)
{
    arglex_init(argc, argv, 0);

    char buffer[2000];
    buffer[0] = 0;
    char *bp = buffer;
    while (arglex_get_string() == arglex_token_string)
    {
	if (buffer[0])
	    bp = strendcpy(bp, " ", buffer + sizeof(buffer));
	bp = strendcpy(bp, arglex_value.alv_string, buffer + sizeof(buffer));
    }
    if (!buffer[0])
	usage();

    //
    // Attempt to convert the string into a time
    //
    time_t when = date_scan(buffer);
    if (when == (time_t)(-1))
	fatal_raw("string \"%s\" is not a valid date or time", buffer);
    printf("%ld\n", (long)when);
    printf("%s\n", date_string(when)); // GMT
    printf("%s", ctime(&when)); // local

    //
    // Report success.
    //
    exit(0);
    return 0;
}
