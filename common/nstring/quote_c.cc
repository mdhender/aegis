//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller
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
// MANIFEST: implementation of the nstring::quote_c method
//

#include <common/ac/ctype.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


nstring
nstring::quote_c()
    const
{
    static nstring_accumulator ac;
    ac.clear();
    ac.push_back('"');
    const char *cp = c_str();
    for (;;)
    {
	unsigned char c = *cp++;
	switch (c)
	{
	case '\0':
	    ac.push_back('"');
	    return ac.mkstr();

	case '\\':
	    ac.push_back("\\\\");
	    break;

	case '"':
	    ac.push_back("\\\"");
	    break;

	case '\a':
	    ac.push_back("\\a");
	    break;

	case '\b':
	    ac.push_back("\\b");
	    break;

	case '\f':
	    ac.push_back("\\f");
	    break;

	case '\n':
	    ac.push_back("\\n");
	    break;

	case '\r':
	    ac.push_back("\\r");
	    break;

	case '\v':
	    ac.push_back("\\v");
	    break;

	case ' ': case '!':           case '#':
	case '$': case '%': case '&': case '\'':
	case '(': case ')': case '*': case '+':
	case ',': case '-': case '.': case '/':
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	case '8': case '9': case ':': case ';':
	case '<': case '=': case '>': case '?':
	case '@': case 'A': case 'B': case 'C':
	case 'D': case 'E': case 'F': case 'G':
	case 'H': case 'I': case 'J': case 'K':
	case 'L': case 'M': case 'N': case 'O':
	case 'P': case 'Q': case 'R': case 'S':
	case 'T': case 'U': case 'V': case 'W':
	case 'X': case 'Y': case 'Z': case '[':
	          case ']': case '^': case '_':
	case '`': case 'a': case 'b': case 'c':
	case 'd': case 'e': case 'f': case 'g':
	case 'h': case 'i': case 'j': case 'k':
	case 'l': case 'm': case 'n': case 'o':
	case 'p': case 'q': case 'r': case 's':
	case 't': case 'u': case 'v': case 'w':
	case 'x': case 'y': case 'z': case '{':
	case '|': case '}': case '~':
	    ac.push_back(c);
	    break;

	default:
	    if (isprint(c))
		ac.push_back(c);
	    else
		ac.push_back(nstring::format("\\%03o", c));
	    break;
	}
    }
}
