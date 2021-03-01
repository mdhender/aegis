//
//	aegis - project change supervisor
//	Copyright (C) 1997-1999, 2002, 2004-2006, 2008 Peter Miller
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

#include <common/gmatch.h>
#include <common/nstring.h>
#include <common/trace.h>


//
// NAME
//	gmatch - match entryname pattern
//
// SYNOPSIS
//	int gmatch(char *formal, char *actual);
//
// DESCRIPTION
//	The formal strings is used as a template to match the given actual
//	string against.
//
//	The pattern elements understood are
//		*	match zero or more of any character
//		?	match any single character
//		[^xxx]	match any single character not in the set given.
//		[xxx]	match any single character in the set given.
//			The - character is understood to be a range indicator.
//			If the ] character is the first of the set it is
//			considered as part of the set, not the terminator.
//
// RETURNS
//	the gmatch function returns zero if they do not match,
//	and nonzero if they do.  Returns -1 on error.
//
// CAVEAT
//	This is a limited set of the sh(1) patterns.
//	Assumes that the `original' global variable has been initialized,
//	it is used for error reporting.
//

int
gmatch(const char *formal, const char *actual)
{
    const char      *cp;
    int		    result;

    trace(("gmatch(formal = %8.8lX, actual = %8.8lX)\n{\n", (long)formal,
	(long)actual));
    while (*formal)
    {
	trace(("formal == \"%s\";\n", formal));
	trace(("actual = \"%s\";\n", actual));
	switch (*formal)
	{
	default:
	    if (*actual++ != *formal++)
	    {
		result = 0;
		goto ret;
	    }
	    break;

	case '?':
	    if (!*actual++)
	    {
		result = 0;
		goto ret;
	    }
	    ++formal;
	    break;

	case '*':
	    for (;;)
	    {
		++formal;
		switch (*formal)
		{
		case 0:
		    result = 1;
		    goto ret;

		case '*':
		    continue;

		case '?':
		    if (!*actual++)
		    {
		       	result = 0;
		       	goto ret;
		    }
		    continue;

		default:
		    break;
		}
		break;
	    }
	    cp = actual + strlen(actual);
	    for (;;)
	    {
		result = gmatch(formal, cp);
		if (result)
		{
		    result = 1;
		    break;
		}
		--cp;
		if (cp < actual)
		{
		    result = 0;
		    break;
		}
	    }
	    goto ret;

	case '[':
	    ++formal;
	    if (*formal == '^')
	    {
		++formal;
		for (;;)
		{
		    if (!*formal)
		    {
		       	no_close:
		       	result = -1;
		       	goto ret;
		    }

		    //
		    // note: this allows leading close
		    // square bracket elegantly
		    //
		    if
		    (
			formal[1] == '-'
		    &&
			formal[2]
		    &&
		       	formal[2] != ']'
		    &&
			formal[3]
		    )
		    {
			char            c1;
			char            c2;

			c1 = formal[0];
			c2 = formal[2];
			formal += 3;
			if
			(
			    c1 <= c2
			?
			    (c1 <= *actual && *actual <= c2)
			:
			    (c2 <= *actual && *actual <= c1)
			)
			{
			    result = 0;
			    goto ret;
			}
		    }
		    else if (*actual == *formal++)
		    {
			result = 0;
			goto ret;
		    }
		    if (*formal == ']')
			break;
		}
		++formal;
	    }
	    else
	    {
		for (;;)
		{
		    if (!*formal)
			goto no_close;

		    //
		    // note: this allows leading close
		    // square bracket elegantly
		    //
		    trace(("formal == \"%s\";\n", formal));
		    trace(("actual = \"%s\";\n", actual));
		    if
		    (
			formal[1] == '-'
		    &&
			formal[2]
		    &&
			formal[2] != ']'
		    &&
			formal[3]
		    )
		    {
			char            c1;
			char            c2;

			c1 = formal[0];
			c2 = formal[2];
			formal += 3;
			if
			(
			    c1 <= c2
			?
			    (c1 <= *actual && *actual <= c2)
			:
			    (c2 <= *actual && *actual <= c1)
			)
			    break;
		    }
		    else if (*actual == *formal++)
			break;
		    if (*formal == ']')
		    {
			result = 0;
			goto ret;
		    }
		}
		for (;;)
		{
		    if (!*formal)
		       	goto no_close;
		    trace(("formal == \"%s\";\n", formal));
		    trace(("actual = \"%s\";\n", actual));
		    if (*formal++ == ']')
		       	break;
		}
	    }
	    ++actual;
	    break;
	}
    }
    result = (*actual == 0);

    //
    // here for all exits
    //
    ret:
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
gmatch(const nstring &pattern, const nstring &candidate)
{
    return gmatch(pattern.c_str(), candidate.c_str());
}
