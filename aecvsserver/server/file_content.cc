//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate file_contents
//

#include <ac/stdlib.h>

#include <input/gunzip.h>
#include <server.h>
#include <net.h>


input_ty *
server_file_contents_get(server_ty *sp)
{
    string_ty       *s;
    long            length;
    int             gunzip;
    const char      *cp;
    char            *end;
    input_ty        *ip;

    gunzip = 0;
    s = server_getline(sp);
    if (!s)
	goto yuck;
    cp = s->str_text;
    if (*cp == 'z')
    {
	gunzip = 1;
	++cp;
    }
    length = strtol(cp, &end, 10);
    if (end == cp || *end || length < 0)
    {
	yuck:
	server_error(sp, "file length \"%s\" invalid", (s ? s->str_text : ""));
	if (s)
	    str_free(s);
	length = 0;
    }
    ip = sp->np->in_crop(length);
    if (gunzip)
	ip = input_gunzip(ip);
    return ip;
}
