//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/main.h>
#include <common/nstring.h>
#include <common/nstring/accumulator.h>


struct table_t
{
    const char *name;
    const char *value;
};

//
// The object of this table is not to be an accurate translation, but to
// simply make it readable.  Most of the time, non-printing characters
// will be transmitted as &#DDD; escapes, and so will unpack at this end
// correctly.  This table is simply for foreign text not produced by
// Aegis.
//
// The table is sorted via "LANG=en sort -u" because we are going to
// binary chop across it for efficiency.
//
static const table_t table[] =
{
    { "AElig", "AE" },
    { "Aacute", "A" },
    { "Acirc", "A" },
    { "Agrave", "A" },
    { "Aring", "A" },
    { "Atilde", "A" },
    { "Auml", "AE" },
    { "Ccedil", "C" },
    { "Eacute", "E" },
    { "Ecirc", "E" },
    { "Egrave", "E" },
    { "Euml", "EE" },
    { "Iacute", "I" },
    { "Icirc", "I" },
    { "Igrave", "I" },
    { "Iuml", "IE" },
    { "Ntilde", "N" },
    { "OElig", "OE" },
    { "Oacute", "O" },
    { "Ocirc", "O" },
    { "Ograve", "O" },
    { "Oslash", "O" },
    { "Otilde", "O" },
    { "Ouml", "OE" },
    { "Scaron", "S" },
    { "Uacute", "U" },
    { "Ucirc", "U" },
    { "Ugrave", "U" },
    { "Uuml", "UE" },
    { "Yacute", "Y" },
    { "Yuml", "YE" },
    { "aacute", "a" },
    { "acirc", "a" },
    { "acute", "a" },
    { "aelig", "ae" },
    { "agrave", "a" },
    { "amp", "&" },
    { "aring", "a" },
    { "atilde", "a" },
    { "auml", "ae" },
    { "bdquo", ",," },
    { "brvbar", "|" },
    { "ccedil", "c" },
    { "cedil", "," },
    { "cent", "c" },
    { "circ", "^" },
    { "copy", "(C)" },
    { "curren", "$" },
    { "deg", "o" },
    { "divide", "/" },
    { "eacute", "e" },
    { "ecirc", "e" },
    { "egrave", "e" },
    { "emsp", " " },
    { "ensp", " " },
    { "euml", "ee" },
    { "frac12", " 1/2 " },
    { "frac14", " 1/4 " },
    { "frac34", " 3/4 " },
    { "gt", ">" },
    { "iacute", "i" },
    { "icirc", "i" },
    { "iexcl", "!" },
    { "igrave", "i" },
    { "iquest", "?" },
    { "iuml", "ie" },
    { "laquo", "<" },
    { "ldquo", "\"" },
    { "lsaquo", "<" },
    { "lsquo", "'" },
    { "lt", "<" },
    { "mdash", "-" },
    { "micro", "u" },
    { "middot", "." },
    { "nbsp", " " },
    { "ndash", "-" },
    { "ntilde", "n" },
    { "oacute", "o" },
    { "ocirc", "o" },
    { "oelig", "oe" },
    { "ograve", "o" },
    { "oslash", "o" },
    { "otilde", "o" },
    { "ouml", "oe" },
    { "permil", "o/oo" },
    { "plusmn", "+/-" },
    { "pound", "L" },
    { "quot", "'" },
    { "quot", "\"" },
    { "raquo", ">" },
    { "rdquo", "\"" },
    { "reg", "(R)" },
    { "rsaquo", ">" },
    { "rsquo", "'" },
    { "sbquo", "," },
    { "scaron", "s" },
    { "sup1", "1" },
    { "sup2", "2" },
    { "sup3", "3" },
    { "szlig", "ss" },
    { "thinsp", " " },
    { "tilde", "~" },
    { "times", "x" },
    { "uacute", "u" },
    { "ucirc", "u" },
    { "ugrave", "u" },
    { "uuml", "ue" },
    { "yacute", "y" },
    { "yuml", "ye" },
    { "zwj", "" },
    { "zwnj", "" },
};


static int
compare(const void *vkey, const void *velem)
{
    const char *key = (const char *)vkey;
    const table_t *elem = (const table_t *)velem;
    return strcmp(key, elem->name);
}


nstring
nstring::html_unquote()
    const
{
    static nstring_accumulator name;
    static nstring_accumulator output;
    output.clear();
    const char *sp = c_str();
    for (;;)
    {
	unsigned char c = *sp++;
	if (!c)
	    break;
	if (c != '&')
	{
	    output.push_back(c);
	    continue;
	}
	c = *sp++;
	if (!c)
	{
	    output.push_back('&');
	    break;
	}
	name.clear();
	if (c == '#')
	{
	    name.push_back(c);
	    int n = 0;
	    for (;;)
	    {
		c = *sp++;
		if (!isdigit(c))
		    break;
		name.push_back(c);
		n = n * 10 + c - '0';
	    }
	    if (c != ';' || name.size() < 2)
	    {
		--sp;
		output.push_back('&');
		output.push_back(name);
	    }
	    else
	    {
		if (n > 0 && n < 256)
		{
		    output.push_back((char)n);
		}
		else
		{
		    output.push_back('&');
		    output.push_back(name);
		    output.push_back(';');
		}
	    }
	}
	else
	{
	    for (;;)
	    {
		if (!isalnum(c))
		    break;
		name.push_back(c);
		c = *sp++;
	    }
	    if (c != ';' || name.empty())
	    {
		--sp;
		output.push_back('&');
		output.push_back(name);
	    }
	    else
	    {
		name.push_back('\0'); // terminating NUL character
		table_t *tp =
		    (table_t *)
		    bsearch
		    (
			name.get_data(),
			table,
			SIZEOF(table),
			sizeof(table[0]),
			compare
		    );
		if (tp)
		{
		    output.push_back(tp->value);
		}
		else
		{
		    name.pop_back(); // get rid of the NUL character
		    output.push_back('&');
		    output.push_back(name);
		    output.push_back(';');
		}
	    }
	}
    }
    return output.mkstr();
}
