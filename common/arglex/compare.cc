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

#include <common/arglex.h>
#include <common/trace.h>


bool
arglex_compare(const char *formal, const char *actual, const char **partial)
{
    trace(("arglex_compare(formal = \"%s\", actual = \"%s\")\n{\n",
	formal, actual));
    bool result = false;
    for (;;)
    {
	trace_string(formal);
	trace_string(actual);
	unsigned char ac = *actual++;
	if (isupper((unsigned char)ac))
	    ac = tolower(ac);
	unsigned char fc = *formal++;
	switch (fc)
	{
	case 0:
	    result = !ac;
	    goto done;

	case '_':
	    if (ac == '-')
		break;
	    // fall through...

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	    //
	    // optional characters
	    //
	    if (ac == fc && arglex_compare(formal, actual, partial))
	    {
		result = true;
		goto done;
	    }

	    //
	    // skip forward to next
	    // mandatory character, or after '_'
	    //
	    while (islower((unsigned char)*formal))
		++formal;
	    if (*formal == '_')
	    {
		++formal;
		if (ac == '_' || ac == '-')
		    ++actual;
	    }
	    --actual;
	    break;

	case '*':
	    //
	    // This is a hack, it should really
	    // check for a match to the stuff after
	    // the '*', too, a la glob.
	    //
	    if (!ac)
	    {
		result = false;
		goto done;
	    }
	    if (partial)
		*partial = actual - 1;
	    result = true;
	    goto done;

	case '\\':
	    if (actual[-1] != *formal++)
	    {
		result = false;
		goto done;
	    }
	    break;

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	    fc = tolower(fc);
	    // fall through...

	default:
	    //
	    // mandatory characters
	    //
	    if (fc != ac)
	    {
		result = false;
		goto done;
	    }
	    break;
	}
    }
  done:
    trace(("return %s;\n}\n", (result ? "true" : "false")));
    return result;
}
