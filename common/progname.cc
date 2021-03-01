//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1999, 2003-2006, 2008 Peter Miller
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

#include <common/exeext.h>
#include <common/progname.h>


static const char *progname;


void
progname_set(char *s)
{
    //
    // do NOT put tracing in this function
    // do NOT put asserts in this function
    //	they both depend on progname, which is not yet set
    //
    for (;;)
    {
	char *slash = strrchr(s, '/');

	//
	// we were invoked as
	//	progname -args
	//
	if (!slash)
	{
	    //
	    // Nuke any ugly progname suffix
	    // if it has one.
	    //
	    int n = exeext(s);
	    if (n > 0)
		s[n] = 0;
	    progname = s;
	    break;
	}

	//
	// we were invoked as
	//	/usr/local/progname -args
	//
	if (slash[1])
	{
	    progname = slash + 1;
	    break;
	}

	//
	// this is real nasty:
	// it is possible to invoke us as
	//	/usr//local///bin////progname///// -args
	// and it is legal!!
	//
	*slash = 0;
    }
}


const char *
progname_get(void)
{
    // do NOT put tracing in this function
    return (progname ? progname : "");
}
