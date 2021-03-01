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
#include <common/ac/string.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


nstring
nstring::url_quote()
    const
{
    static nstring_accumulator sa;
    sa.clear();
    const char *sp = c_str();
    for (;;)
    {
	unsigned char c = *sp++;
	if (!c)
	    break;
	// C locale
	if (!isprint(c) || strchr(" \"#%&'+:=?~^|$(){}[]", c))
	{
	    //
	    //	    #:?	    URL special characters
	    //	    %	    escape the escape character
	    //	    ~	    illegal in portable character sets
	    //	    &=	    confuse forms-based browsers
	    //	    "'	    I've included some shell meta
	    //      ${}[]   characters here, too, because
	    //	    ^|()    these are usually passed through
	    //		    shell scripts.
	    //
	    static char	hex[] =	"0123456789ABCDEF";
	    sa.push_back('%');
	    sa.push_back(hex[(c >> 4) & 15]);
	    sa.push_back(hex[c & 15]);
	}
	else
	    sa.push_back(c);
    }
    return sa.mkstr();
}
