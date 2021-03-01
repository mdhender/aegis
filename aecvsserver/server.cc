//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/mem.h>
#include <libaegis/os.h>

#include <aecvsserver/file_info.h>
#include <aecvsserver/response/error.h>
#include <aecvsserver/response/e.h>
#include <aecvsserver/response/m.h>
#include <aecvsserver/response/ok.h>
#include <aecvsserver/server/private.h>


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
server_response_queue(server_ty *sp, response *rp)
{
    sp->np->response_queue(rp);
}


void
server_response_flush(server_ty *sp)
{
    sp->np->response_flush();
}


bool
server_getline(server_ty *sp, nstring &result)
{
    return sp->np->getline(result);
}


void
server_ok(server_ty *sp)
{
    assert(sp);
    server_response_queue(sp, new response_ok());
}


void
server_error(server_ty *sp, const char *fmt, ...)
{
    assert(sp);
    assert(fmt);
    va_list ap;
    va_start(ap, fmt);
    string_ty *msg = str_vformat(fmt, ap);
    va_end(ap);

    server_response_queue(sp, new response_error(msg));
}


void
server_e(server_ty *sp, const char *fmt, ...)
{
    assert(sp);
    assert(fmt);
    va_list ap;
    va_start(ap, fmt);
    string_ty *msg = str_vformat(fmt, ap);
    va_end(ap);

    server_response_queue(sp, new response_e(msg));
}


void
server_m(server_ty *sp, const char *fmt, ...)
{
    assert(sp);
    assert(fmt);
    va_list ap;
    va_start(ap, fmt);
    string_ty *msg = str_vformat(fmt, ap);
    va_end(ap);

    server_response_queue(sp, new response_m(msg));
}


int
server_root_required(server_ty *sp, const char *caption)
{
    int             root_required;

    assert(sp);
    assert(caption);
    root_required = !sp->np->get_is_rooted();
    if (root_required)
        server_error(sp, "%s: must send Root request first", caption);
    return root_required;
}


int
server_directory_required(server_ty *sp, const char *caption)
{
    assert(sp);
    assert(caption);
    bool bad = !sp->np->curdir_is_set();
    if (bad)
        server_error(sp, "%s: must send Directory request first", caption);
    return bad;
}


void
server_argument(server_ty *sp, string_ty *arg)
{
    assert(sp);
    assert(sp->np);
    sp->np->argument(arg);
}


void
server_argumentx(server_ty *sp, string_ty *arg)
{
    assert(sp);
    assert(sp->np);
    sp->np->argumentx(arg);
}


void
server_accumulator_reset(server_ty *sp)
{
    sp->np->accumulator_reset();
}


file_info_ty *
server_file_info_find(server_ty *sp, string_ty *server_side, int auto_alloc)
{
    return sp->np->file_info_find(server_side, auto_alloc);
}


void
server_updating_verbose(server_ty *sp, string_ty *client_side)
{
    string_ty *dir = os_dirname_relative(client_side);
    if
    (
        // this is a string, not a bool
        !sp->np->get_updating_verbose()
    ||
        !str_equal(dir, sp->np->get_updating_verbose())
    )
    {
        server_e(sp, "Updating %s", dir->str_text);
        sp->np->set_updating_verbose(dir);
    }
    str_free(dir);
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
            dp = sp->np->directory_find_server_side(ss);
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


// vim: set ts=8 sw=4 et :
