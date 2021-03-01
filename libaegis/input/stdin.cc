//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2006, 2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>
#include <sys/socket.h>
#include <common/ac/unistd.h>

#include <libaegis/input/stdin.h>
#include <libaegis/sub.h>


input_stdin::~input_stdin()
{
}


input_stdin::input_stdin() :
    pos(0),
    unbuffered(false)
{
}


static nstring
standard_input(void)
{
    static nstring cache;
    if (cache.empty())
    {
	sub_context_ty sc;
	cache = nstring(sc.subst_intl(i18n("standard input")));
    }
    return cache;
}


long
input_stdin::read_inner(void *data, size_t len)
{
    if (len <= 0)
	return 0;
    if (unbuffered)
	len = 1;
    int fd = fileno(stdin);
    long result = ::read(fd, data, len);
    if (result < 0)
    {
	int errno_old = errno;
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", standard_input());
	sc.fatal_intl(i18n("read $filename: $errno"));
	// NOTREACHED
    }
    pos += result;
    return result;
}


long
input_stdin::ftell_inner()
{
    return pos;
}


nstring
input_stdin::name()
{
    return standard_input();
}


long
input_stdin::length()
{
    struct stat st;
    if (fstat(fileno(stdin), &st) < 0)
	return -1;
    if (!S_ISREG(st.st_mode))
	return -1;
    return st.st_size;
}


void
input_stdin::keepalive()
{
    //
    // This method is a hint that we are talking to a network socket,
    // so to get an error fairly soon after the other end drops out,
    // set the keepalive option.
    //
    int fd = fileno(stdin);
    int on = 1;
    // ignore any error
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));

    //
    // Sucking on pipes (pretending to be network sockets) plays merry
    // hell with line buffering, so don't read big blocks in this case
    // (it blocks to read all the data you asked for, rather than being
    // interactive).
    //
    unbuffered = true;
}
