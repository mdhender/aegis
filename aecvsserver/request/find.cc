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
// MANIFEST: functions to manipulate finds
//

#include <request/add.h>
#include <request/admin.h>
#include <request/argument.h>
#include <request/argumentx.h>
#include <request/ci.h>
#include <request/co.h>
#include <request/directory.h>
#include <request/entry.h>
#include <request/expand_modul.h>
#include <request/global_optio.h>
#include <request/init.h>
#include <request/is_modified.h>
#include <request/modified.h>
#include <request/noop.h>
#include <request/questionable.h>
#include <request/remove.h>
#include <request/repository.h>
#include <request/root.h>
#include <request/set.h>
#include <request/unchanged.h>
#include <request/update.h>
#include <request/useunchanged.h>
#include <request/unknown.h>
#include <request/valid_respon.h>
#include <request/version.h>
#include <response/valid_reques.h>
#include <server.h>
#include <symtab.h>


static void valid_requests_run(server_ty *, string_ty *);


static request_ty request_valid_requests =
{
    "valid-requests",
    valid_requests_run,
    1, // reset
};

static const request_ty *const table[] =
{
    &request_add,
    &request_admin,
    &request_argument,
    &request_argumentx,
    &request_ci,
    &request_co,
    &request_directory,
    &request_entry,
    &request_expand_modules,
    &request_global_option,
    &request_init,
    &request_is_modified,
    &request_modified,
    &request_noop,
    &request_questionable,
    &request_remove,
    &request_repository,
    &request_root,
    &request_set,
    &request_unchanged,
    &request_update,
    &request_useunchanged,
    &request_valid_requests,
    &request_valid_responses,
    &request_version,
};


const request_ty *
request_find(string_ty *name)
{
    const request_ty *rp;
    static symtab_ty *stp;

    if (!stp)
    {
	const request_ty *const *tpp;

	stp = symtab_alloc(SIZEOF(table));
	for (tpp = table; tpp < ENDOF(table); ++tpp)
	{
	    const request_ty *tp;
	    string_ty       *key;

	    tp = *tpp;
	    key = str_from_c(tp->name);
	    symtab_assign(stp, key, (void *)tp);
	    str_free(key);
	}
    }
    rp = (request_ty *)symtab_query(stp, name);
    if (!rp)
	rp = request_unknown(name->str_text);
    return rp;
}


//
// Ask the server to send back a "Valid-requests" response,
// listing all the requests this server understands.
//
// Response expected: yes.
// Root required: no.
//

static void
valid_requests_run(server_ty *sp, string_ty *arg)
{
    string_list_ty  wl;
    const request_ty *const *tpp;

    for (tpp = table; tpp < ENDOF(table); ++tpp)
    {
	const request_ty *tp;
	string_ty       *key;

	tp = *tpp;
	key = str_from_c(tp->name);
	wl.push_back(key);
	str_free(key);
    }
    server_response_queue(sp, response_valid_requests_new(&wl));
    server_ok(sp);
}
