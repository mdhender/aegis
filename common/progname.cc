//
//	aegis - project change supervisor
//	Copyright (C) 1995, 1999, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate the program name
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
