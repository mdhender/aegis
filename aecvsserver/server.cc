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
// MANIFEST: functions to manipulate servers
//

#include <error.h> // for assert
#include <file_info.h>
#include <mem.h>
#include <os.h>
#include <response/error.h>
#include <response/e.h>
#include <response/m.h>
#include <response/ok.h>
#include <server/private.h>


void
server_run(server_ty *sp)
{
    assert(sp);
    assert(sp->vptr);
    assert(sp->vptr->run);
    sp->vptr->run(sp);
}


void
server_delete(server_ty *sp)
{
    assert(sp);
    assert(sp->vptr);
    if (sp->vptr->destructor)
	sp->vptr->destructor(sp);
    sp->vptr = 0;
    sp->np = 0;
    mem_free(sp);
}


void
server_response_queue(server_ty *sp, response_ty *rp)
{
    net_response_queue(sp->np, rp);
}


void
server_response_flush(server_ty *sp)
{
    net_response_flush(sp->np);
}


string_ty *
server_getline(server_ty *sp)
{
    return net_getline(sp->np);
}


void
server_ok(server_ty *sp)
{
    assert(sp);
    server_response_queue(sp, response_ok_new());
}


void
server_error(server_ty *sp, const char *fmt, ...)
{
    va_list         ap;

    assert(sp);
    assert(fmt);
    va_start(ap, fmt);
    server_response_queue(sp, response_error_new_v(fmt, ap));
    va_end(ap);
}


void
server_e(server_ty *sp, const char *fmt, ...)
{
    va_list         ap;

    assert(sp);
    assert(fmt);
    va_start(ap, fmt);
    server_response_queue(sp, response_e_new_v(fmt, ap));
    va_end(ap);
}


void
server_m(server_ty *sp, const char *fmt, ...)
{
    va_list         ap;

    assert(sp);
    assert(fmt);
    va_start(ap, fmt);
    server_response_queue(sp, response_m_new_v(fmt, ap));
    va_end(ap);
}


int
server_root_required(server_ty *sp, const char *caption)
{
    int             root_required;

    assert(sp);
    assert(caption);
    root_required = !sp->np->rooted;
    if (root_required)
	server_error(sp, "%s: must send Root request first", caption);
    return root_required;
}


int
server_directory_required(server_ty *sp, const char *caption)
{
    int             bad;

    assert(sp);
    assert(caption);
    bad = (sp->np->curdir == 0);
    if (bad)
	server_error(sp, "%s: must send Directory request first", caption);
    return bad;
}


void
server_argument(server_ty *sp, string_ty *arg)
{
    assert(sp);
    assert(sp->np);
    net_argument(sp->np, arg);
}


void
server_argumentx(server_ty *sp, string_ty *arg)
{
    assert(sp);
    assert(sp->np);
    net_argumentx(sp->np, arg);
}


void
server_accumulator_reset(server_ty *sp)
{
    net_accumulator_reset(sp->np);
}


file_info_ty *
server_file_info_find(server_ty *sp, string_ty *server_side, int auto_alloc)
{
    return net_file_info_find(sp->np, server_side, auto_alloc);
}


void
server_updating_verbose(server_ty *sp, string_ty *client_side)
{
    string_ty       *dir;

    dir = os_dirname_relative(client_side);
    if (sp->np->updating_verbose)
    {
	if (str_equal(dir, sp->np->updating_verbose))
	{
	    str_free(dir);
	    return;
	}
	str_free(sp->np->updating_verbose);
    }
    server_e(sp, "Updating %s", dir->str_text);
    sp->np->updating_verbose = dir;
}


string_ty *
server_directory_calc_client_side(server_ty *sp, string_ty *server_side)
{
    const char      *start;
    const char      *end;
    const char      *slash;

    start = server_side->str_text;
    end = start + server_side->str_length;
    slash = end;
    while (slash > start)
    {
	--slash;
	if (*slash == '/')
	{
	    string_ty       *ss;
	    directory_ty    *dp;

	    ss = str_n_from_c(start, end - start);
	    dp = net_directory_find_server_side(sp->np, ss);
	    str_free(ss);
	    if (dp)
	    {
		string_ty       *tmp;
		string_ty       *cs;

		tmp = str_n_from_c(slash, end - slash);
		cs = str_catenate(dp->client_side, tmp);
		str_free(tmp);
		return cs;
	    }
	}
    }
    return str_copy(server_side);
}
