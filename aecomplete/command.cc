//
//	aegis - project change supervisor
//	Copyright (C) 2002-2006, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <aecomplete/command/aeb.h>
#include <aecomplete/command/aeca.h>
#include <aecomplete/command/aecd.h>
#include <aecomplete/command/ae_c.h>
#include <aecomplete/command/aechown.h>
#include <aecomplete/command/aeclean.h>
#include <aecomplete/command/aecp.h>
#include <aecomplete/command/aecpu.h>
#include <aecomplete/command/aed.h>
#include <aecomplete/command/aedb.h>
#include <aecomplete/command/aedbu.h>
#include <aecomplete/command/aede.h>
#include <aecomplete/command/aedeu.h>
#include <aecomplete/command/aedn.h>
#include <aecomplete/command/aefa.h>
#include <aecomplete/command/aegis.h>
#include <aecomplete/command/aeib.h>
#include <aecomplete/command/aeibu.h>
#include <aecomplete/command/aeifail.h>
#include <aecomplete/command/aeipass.h>
#include <aecomplete/command/aemt.h>
#include <aecomplete/command/aemtu.h>
#include <aecomplete/command/aena.h>
#include <aecomplete/command/aencu.h>
#include <aecomplete/command/aend.h>
#include <aecomplete/command/aenf.h>
#include <aecomplete/command/aenfu.h>
#include <aecomplete/command/aeni.h>
#include <aecomplete/command/aenrv.h>
#include <aecomplete/command/aentu.h>
#include <aecomplete/command/aepa.h>
#include <aecomplete/command/ae_p.h>
#include <aecomplete/command/aera.h>
#include <aecomplete/command/aerb.h>
#include <aecomplete/command/aerbu.h>
#include <aecomplete/command/aerd.h>
#include <aecomplete/command/aerfail.h>
#include <aecomplete/command/aeri.h>
#include <aecomplete/command/aerm.h>
#include <aecomplete/command/aermu.h>
#include <aecomplete/command/aerpass.h>
#include <aecomplete/command/aerpu.h>
#include <aecomplete/command/aerrv.h>
#include <aecomplete/command/aet.h>
#include <aecomplete/command.h>
#include <aecomplete/command/private.h>
#include <aecomplete/command/unknown.h>
#include <aecomplete/complete.h>
#include <common/error.h> // for assert
#include <common/symtab.h>


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
