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

#include <common/ac/stdlib.h>
#include <libaegis/input/gunzip.h>
#include <libaegis/input/null.h>

#include <aecvsserver/server.h>
#include <aecvsserver/net.h>


input
server_file_contents_get(server_ty *sp)
{
    nstring s;
    if (!server_getline(sp, s))
	return new input_null();
    const char *cp = s.c_str();
    bool gunzip = false;
    if (*cp == 'z')
    {
	gunzip = true;
	++cp;
    }
    char *end = 0;
    long length = strtol(cp, &end, 10);
    if (end == cp || *end || length < 0)
    {
	server_error(sp, "file length \"%s\" invalid", s.c_str());
	return new input_null();
    }
    input ip = sp->np->in_crop(length);
    if (gunzip)
	ip = new input_gunzip(ip);
    return ip;
}
