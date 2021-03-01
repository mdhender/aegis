//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate commands
//

#include <ac/string.h>

#include <command/aeb.h>
#include <command/aeca.h>
#include <command/aecd.h>
#include <command/ae_c.h>
#include <command/aechown.h>
#include <command/aeclean.h>
#include <command/aecp.h>
#include <command/aecpu.h>
#include <command/aed.h>
#include <command/aedb.h>
#include <command/aedbu.h>
#include <command/aede.h>
#include <command/aedeu.h>
#include <command/aedn.h>
#include <command/aefa.h>
#include <command/aegis.h>
#include <command/aeib.h>
#include <command/aeibu.h>
#include <command/aeifail.h>
#include <command/aeipass.h>
#include <command/aemt.h>
#include <command/aemtu.h>
#include <command/aena.h>
#include <command/aencu.h>
#include <command/aend.h>
#include <command/aenf.h>
#include <command/aenfu.h>
#include <command/aeni.h>
#include <command/aenrv.h>
#include <command/aentu.h>
#include <command/aepa.h>
#include <command/ae_p.h>
#include <command/aera.h>
#include <command/aerb.h>
#include <command/aerbu.h>
#include <command/aerd.h>
#include <command/aerfail.h>
#include <command/aeri.h>
#include <command/aerm.h>
#include <command/aermu.h>
#include <command/aerpass.h>
#include <command/aerpu.h>
#include <command/aerrv.h>
#include <command/aet.h>
#include <command.h>
#include <command/private.h>
#include <command/unknown.h>
#include <complete.h>
#include <error.h> // for assert
#include <symtab.h>


typedef command_ty *(*funcptr)(void);

static funcptr table[] =
{
    command_aeb,
    command_ae_c,
    command_aeca,
    command_aecd,
    command_aechown,
    command_aeclean,
    command_aecp,
    command_aecpu,
    command_aed,
    command_aedb,
    command_aedbu,
    command_aede,
    command_aedeu,
    command_aedn,
    command_aefa,
    command_aegis,
    command_aeib,
    command_aeibu,
    command_aemt,
    command_aemtu,
    command_aena,
    command_aencu,
    command_aend,
    command_aenf,
    command_aenfu,
    command_aeni,
    command_aenrv,
    command_aentu,
    command_ae_p,
    command_aepa,
    command_aera,
    command_aerb,
    command_aerbu,
    command_aerd,
    command_aerfail,
    command_aeri,
    command_aerm,
    command_aermu,
    command_aerpass,
    command_aerpu,
    command_aerrv,
    command_aet,
};

static symtab_ty *stp;


command_ty *
command_find(string_ty *name)
{
    command_ty      *cp;
    funcptr         *tp;
    funcptr         func;
    string_ty       *s;

    if (!stp)
    {
	stp = symtab_alloc(SIZEOF(table));
	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    func = *tp;
	    cp = func();
	    s = str_from_c(command_name(cp));
	    symtab_assign(stp, s, cp);
	    str_free(s);
	}
    }
    cp = (command_ty *)symtab_query(stp, name);
    if (cp)
	return cp;
    return command_unknown();
}


const char *
command_name(command_ty *this_thing)
{
    assert(this_thing);
    assert(this_thing->vptr);
    return this_thing->vptr->name;
}


complete_ty *
command_completion_get(command_ty *this_thing)
{
    assert(this_thing);
    assert(this_thing->vptr);
    assert(this_thing->vptr->completion_get);
    return this_thing->vptr->completion_get(this_thing);
}
