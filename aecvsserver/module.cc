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
// MANIFEST: functions to manipulate modules
//

#include <error.h> // for assert
#include <file_info.h>
#include <mem.h>
#include <module/private.h>
#include <server.h>


void
module_delete(module_ty *mp)
{
    assert(mp);
    assert(mp->vptr);
    assert(mp->reference_count >= 1);
    mp->reference_count--;
    if (mp->reference_count <= 0)
    {
	if (mp->vptr->destructor)
	    mp->vptr->destructor(mp);
	mp->vptr = 0; // paranoia
	mem_free(mp);
    }
    if (mp->canonical_name)
    {
	str_free(mp->canonical_name);
	mp->canonical_name = 0;
    }
}


void
module_modified(module_ty *mp, server_ty *sp, string_ty *file_name,
    file_info_ty *fip, input_ty *contents)
{
    assert(mp);
    assert(mp->vptr);
    assert(mp->vptr->modified);
    mp->vptr->modified(mp, sp, file_name, fip, contents);
}


int
module_bogus(module_ty *mp)
{
    assert(mp);
    assert(mp->vptr);
    return mp->vptr->bogus;
}


string_ty *
module_name(module_ty *mp)
{
    assert(mp);
    if (!mp->canonical_name)
    {
	assert(mp->vptr);
	assert(mp->vptr->canonical_name);
	mp->canonical_name = mp->vptr->canonical_name(mp);
    }
    return mp->canonical_name;
}


void
module_checkout(module_ty *mp, server_ty *sp)
{
    module_options_ty opt;
    string_ty       *name;

    module_options_constructor(&opt);
    opt.d = 1;
    opt.C = 1;
    name = module_name(mp);
    if (module_update(mp, sp, name, name, &opt))
	server_ok(sp);
    module_options_destructor(&opt);
}


int
module_checkin(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    assert(mp);
    assert(mp->vptr);
    assert(mp->vptr->checkin);
    return mp->vptr->checkin(mp, sp, client_side, server_side);
}


void
module_options_constructor(module_options_ty *opt)
{
    opt->A = 0;
    opt->C = 0;
    opt->D = 0;
    opt->d = 0;
    opt->f = 0;
    opt->I = 0;
    opt->j = 0;
    opt->k = 0;
    opt->l = 0;
    opt->m = 0;
    opt->P = 0;
    opt->p = 0;
    opt->R = 0;
    opt->r = 0;
    opt->W = 0;
}


void
module_options_destructor(module_options_ty *opt)
{
    opt->A = 0;
    opt->C = 0;
    if (opt->D)
	str_free(opt->D);
    opt->D = 0;
    opt->d = 0;
    opt->f = 0;
    if (opt->I)
	str_free(opt->I);
    opt->I = 0;
    if (opt->j)
	str_free(opt->j);
    opt->j = 0;
    if (opt->k)
	str_free(opt->k);
    opt->k = 0;
    opt->l = 0;
    if (opt->m)
	str_free(opt->m);
    opt->m = 0;
    opt->P = 0;
    opt->p = 0;
    opt->R = 0;
    if (opt->r)
	str_free(opt->r);
    opt->r = 0;
    if (opt->W)
	str_free(opt->W);
    opt->W = 0;
}


int
module_update(module_ty *mp, server_ty *sp, string_ty *cs, string_ty *ss,
    module_options_ty *opt)
{
    assert(mp);
    assert(mp->vptr);
    assert(mp->vptr->checkin);
    return mp->vptr->update(mp, sp, cs, ss, opt);
}


int
module_add(module_ty *mp, server_ty *sp, string_ty *cs, string_ty *ss,
    module_options_ty *opt)
{
    assert(mp);
    assert(mp->vptr);
    assert(mp->vptr->add);
    return mp->vptr->add(mp, sp, cs, ss, opt);
}


int
module_remove(module_ty *mp, server_ty *sp, string_ty *cs, string_ty *ss,
    module_options_ty *opt)
{
    assert(mp);
    assert(mp->vptr);
    assert(mp->vptr->remove);
    return mp->vptr->remove(mp, sp, cs, ss, opt);
}
