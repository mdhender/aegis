//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1999, 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions for reading input from environment variables
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <input/crlf.h>
#include <input/env.h>


input_env::~input_env()
{
}


static const char *
safe_getenv(const char *s)
{
    const char *cp = getenv(s);
    return (cp ? cp : "");
}


input_env::input_env(const nstring &arg) :
    gname(arg),
    base(safe_getenv(arg.c_str())),
    pos(0)
{
}


input_ty *
input_env_open(const char *name)
{
    //
    // You need the CRLF filter, otherwise bizzare things happen on
    // DOS (or DOS-like) operating systems.
    //
    return new input_crlf(new input_env(name), true);
}


long
input_env::read_inner(void *data, size_t len)
{
    if (pos >= base.size())
	return 0;
    size_t nbytes = base.size() - pos;
    if (nbytes > len)
	nbytes = len;
    memcpy(data, base.c_str() + pos, nbytes);
    pos += nbytes;
    return nbytes;
}


long
input_env::ftell_inner()
{
    return pos;
}


nstring
input_env::name()
{
    return gname;
}


long
input_env::length()
{
    return base.size();
}
