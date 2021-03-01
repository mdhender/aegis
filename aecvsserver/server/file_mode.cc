//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/ctype.h>

#include <aecvsserver/server.h>


//
// cvsclient.texi: File Modes
//
// A mode is any number of repetitions of
//	<mode-type> = <data>
// separated by comma (,).
//
// The <mode-type> is an identifier composed of alphanumeric characters.
// Currently specified: "u" for user, "g" for group, "o" for other (see
// below for discussion of whether these have their POSIX meaning or are
// more loose).  Unrecognized values of <mode-type> are silently ignored.
//
// The <data> consists of any data not containing comma (,), end of
// string (\0) or end if line (\n).  For u, g, and o mode types, the
// <data> consists of alphanumeric characters, where "r" means read,
// "w" means write, "x" means execute, and unrecognized letters are
// silently ignored.
//

static int
parse_mode_string(string_ty *s, int *mode_p)
{
    int             mode;
    const char      *cp;
    unsigned char   c;
    int             lhs;
    int             rhs;

    if (!s)
	return 0;
    mode = 0;
    cp = s->str_text;
    for (;;)
    {
	//
        // Question: it says "any number of repetitions", what about zero?
	// This code will accept zero repetitions.
	//
	while (*cp == ',')
	    ++cp;
	if (!*cp)
	{
	    *mode_p = mode;
	    return 1;
	}

	//
	// Get the left hand side.
	// Question: what if it's empty?
	//
	lhs = 0;
	for (;;)
	{
	    c = *cp++;
	    switch (c)
	    {
	    case 'u':
		lhs |= 0700;
		continue;

	    case 'g':
		lhs |= 0070;
		continue;

	    case 'o':
		lhs |= 0007;
		continue;

	    case 0:
		return 0;

	    case '=':
		break;

	    default:
		//
		// The <mode-type> is an identifier composed of
		// alphanumeric characters.  Unrecognized values of
		// <mode-type> are silently ignored.
		//
		if (!isalnum(c))
		    return 0;
		continue;
	    }
	    break;
	}

	//
	// Get the right hand side.
	// Question: what if it's empty?
	//
	rhs = 0;
	for (;;)
	{
	    switch (*cp++)
	    {
	    case 'r':
		rhs |= 0444;
		continue;

	    case 'w':
		rhs |= 0222;
		continue;

	    case 'x':
		rhs |= 0111;
		continue;

	    case '\0':
		--cp;
		break;

	    case '\n':
	    case ',':
		break;
	    }
	    break;
	}

	//
	// Add the appropriate bits to the mode.
	//
	mode |= (lhs & rhs);
    }
}


int
server_file_mode_get(server_ty *sp)
{
    int mode = 0;
    nstring mode_string;
    if
    (
	!server_getline(sp, mode_string)
    ||
	!parse_mode_string(mode_string.get_ref(), &mode)
    )
    {
	server_error(sp, "malformed \"%s\" mode string", mode_string.c_str());
	mode = 0644;
    }
    return mode;
}
