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
// MANIFEST: implementation of the html_url_quote class
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
