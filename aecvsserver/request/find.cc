//
// aegis - project change supervisor
// Copyright (C) 2004-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/sizeof.h>
#include <common/symtab.h>

#include <aecvsserver/request/add.h>
#include <aecvsserver/request/admin.h>
#include <aecvsserver/request/argument.h>
#include <aecvsserver/request/argumentx.h>
#include <aecvsserver/request/ci.h>
#include <aecvsserver/request/co.h>
#include <aecvsserver/request/directory.h>
#include <aecvsserver/request/entry.h>
#include <aecvsserver/request/expand_modul.h>
#include <aecvsserver/request/global_optio.h>
#include <aecvsserver/request/init.h>
#include <aecvsserver/request/is_modified.h>
#include <aecvsserver/request/modified.h>
#include <aecvsserver/request/noop.h>
#include <aecvsserver/request/questionable.h>
#include <aecvsserver/request/remove.h>
#include <aecvsserver/request/repository.h>
#include <aecvsserver/request/root.h>
#include <aecvsserver/request/set.h>
#include <aecvsserver/request/unchanged.h>
#include <aecvsserver/request/update.h>
#include <aecvsserver/request/useunchanged.h>
#include <aecvsserver/request/unknown.h>
#include <aecvsserver/request/valid_reques.h>
#include <aecvsserver/request/valid_respon.h>
#include <aecvsserver/request/version.h>
#include <aecvsserver/server.h>


static request_add add;
static request_admin admin;
static request_argument argument;
static request_argumentx argumentx;
static request_checkin ci;
static request_checkout co;
static request_directory directory;
static request_entry entry;
static request_expand_modules expand_modules;
static request_global_option global_option;
static request_init init;
static request_is_modified is_modified;
static request_modified modified;
static request_noop noop;
static request_questionable questionable;
static request_remove remove_rq;
static request_repository repository;
static request_root root;
static request_set set;
static request_unchanged unchanged;
static request_update update;
static request_useunchanged useunchanged;
static request_valid_requests valid_requests;
static request_valid_responses valid_responses;
static request_version version;

static const request *const table[] =
{
    &add,
    &admin,
    &argument,
    &argumentx,
    &ci,
    &co,
    &directory,
    &entry,
    &expand_modules,
    &global_option,
    &init,
    &is_modified,
    &modified,
    &noop,
    &questionable,
    &remove_rq,
    &repository,
    &root,
    &set,
    &unchanged,
    &update,
    &useunchanged,
    &valid_requests,
    &valid_responses,
    &version,
};


const request *
request::find(string_ty *a_name)
{
    static symtab_ty *stp;
    if (!stp)
    {
        stp = new symtab_ty(SIZEOF(table));
        for (const request *const *tpp = table; tpp < ENDOF(table); ++tpp)
        {
            const request *rp = *tpp;
            string_ty *key = str_from_c(rp->name());
            stp->assign(key, (void *)rp);
            str_free(key);
        }
    }
    const request *rp = (const request *)stp->query(a_name);
    if (!rp)
    {
        rp = new request_unknown(a_name);
        stp->assign(a_name, (void *)rp);
    }
    return rp;
}


void
request::get_list(string_list_ty &result)
{
    for (const request *const *tpp = table; tpp < ENDOF(table); ++tpp)
    {
        const request *rp = *tpp;
        string_ty *key = str_from_c(rp->name());
        result.push_back(key);
        str_free(key);
    }
}


// vim: set ts=8 sw=4 et :
