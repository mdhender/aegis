//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate stdins
//

#include <ac/errno.h>
#include <ac/stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <ac/unistd.h>

#include <input/private.h>
#include <input/stdin.h>
#include <sub.h>
#include <str.h>


struct input_stdin_ty
{
    input_ty        inherited;
    long            pos;
    int             unbuffered;
};


static string_ty *
standard_input(void)
{
    static string_ty *name;
    sub_context_ty  *scp;

    if (!name)
    {
	scp = sub_context_new();
	name = subst_intl(scp, i18n("standard input"));
	sub_context_delete(scp);
    }
    return name;
}


static void
input_stdin_destructor(input_ty *this_thing)
{
}


static long
input_stdin_read(input_ty *ip, void *data, size_t len)
{
    long            result;
    input_stdin_ty  *isp;
    int             fd;

    if (len <= 0)
	return 0;
    isp = (input_stdin_ty *)ip;
    if (isp->unbuffered)
	len = 1;
    fd = fileno(stdin);
    result = read(fd, data, len);
    if (result < 0)
    {
	sub_context_ty	*scp;
	int             errno_old;

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", standard_input());
	fatal_intl(scp, i18n("read $filename: $errno"));
	// NOTREACHED
    }
    isp->pos += result;
    return result;
}


static long
input_stdin_ftell(input_ty *ip)
{
    input_stdin_ty  *isp;

    isp = (input_stdin_ty *)ip;
    return isp->pos;
}


static string_ty *
input_stdin_name(input_ty *this_thing)
{
    return standard_input();
}


static long
input_stdin_length(input_ty *this_thing)
{
    struct stat     st;

    if (fstat(fileno(stdin), &st) < 0)
	    return -1;
    if (!S_ISREG(st.st_mode))
	    return -1;
    return st.st_size;
}


static void
input_stdin_keepalive(input_ty *ip)
{
    input_stdin_ty  *isp;
    int             fd;
    int             on;

    //
    // This method is a hint that we are talking to a network socket,
    // so to get an error fairly soon after the other end drops out,
    // set the keepalive option.
    //
    fd = fileno(stdin);
    on = 1;
    // ignore any error
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));

    //
    // Sucking on pipes (pretending to be network sockets) plays merry
    // hell with line buffering, so don't read big blocks in this case
    // (it blocks to read all the data you asked for, rather than being
    // interactive).
    //
    isp = (input_stdin_ty *)ip;
    isp->unbuffered = 1;
}


static input_vtbl_ty vtbl =
{
    sizeof(input_stdin_ty),
    input_stdin_destructor,
    input_stdin_read,
    input_stdin_ftell,
    input_stdin_name,
    input_stdin_length,
    input_stdin_keepalive,
};


input_ty *
input_stdin(void)
{
    input_ty        *ip;
    input_stdin_ty  *isp;

    ip = input_new(&vtbl);
    isp = (input_stdin_ty *)ip;
    isp->unbuffered = 0;
    isp->pos = 0;
    return ip;
}
