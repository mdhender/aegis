/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate aegiss
 */

#include <arglex2.h>
#include <command/aegis.h>
#include <command/private.h>
#include <complete/nil.h>
#include <complete/project/name.h>
#include <complete/project/name.h>


static complete_ty *result;


static void destructor _((command_ty *));

static void
destructor(this)
    command_ty     *this;
{
}


static void redirect _((const char *));

static void
redirect(name)
    const char     *name;
{
    command_ty     *cmd;
    string_ty      *s;

    s = str_from_c(name);
    cmd = command_find(s);
    str_free(s);
    result = command_completion_get(cmd);
}


static void aeb _((void));

static void
aeb()
{
    redirect("aeb");
}


static void aeca _((void));

static void
aeca()
{
    redirect("aeca");
}


static void aecd _((void));

static void
aecd()
{
    redirect("aecd");
}


static void aechown _((void));

static void
aechown()
{
    redirect("aechown");
}


static void aeclean _((void));

static void
aeclean()
{
    redirect("aeclean");
}


static void aeclone _((void));

static void
aeclone()
{
    redirect("aeclone");
}


static void aecp _((void));

static void
aecp()
{
    redirect("aecp");
}


static void aecpu _((void));

static void
aecpu()
{
    redirect("aecpu");
}


static void aedb _((void));

static void
aedb()
{
    redirect("aedb");
}


static void aedbu _((void));

static void
aedbu()
{
    redirect("aedbu");
}


static void aede _((void));

static void
aede()
{
    redirect("aede");
}


static void aedeu _((void));

static void
aedeu()
{
    redirect("aedeu");
}


static void aed _((void));

static void
aed()
{
    redirect("aed");
}


static void aedn _((void));

static void
aedn()
{
    redirect("aedn");
}


static void aeib _((void));

static void
aeib()
{
    redirect("aeib");
}


static void aeibu _((void));

static void
aeibu()
{
    redirect("aeibu");
}


static void aeifail _((void));

static void
aeifail()
{
    redirect("aeifail");
}


static void aeipass _((void));

static void
aeipass()
{
    redirect("aeipass");
}


static void aemv _((void));

static void
aemv()
{
    redirect("aemv");
}


static void aemvu _((void));

static void
aemvu()
{
    redirect("aemvu");
}


static void aena _((void));

static void
aena()
{
    redirect("aena");
}


static void aenbr _((void));

static void
aenbr()
{
    redirect("aenbr");
}


static void aenbru _((void));

static void
aenbru()
{
    redirect("aenbru");
}


static void aenc _((void));

static void
aenc()
{
    redirect("aenc");
}


static void aencu _((void));

static void
aencu()
{
    redirect("aencu");
}


static void aend _((void));

static void
aend()
{
    redirect("aend");
}


static void aenf _((void));

static void
aenf()
{
    redirect("aenf");
}


static void aenfu _((void));

static void
aenfu()
{
    redirect("aenfu");
}


static void aeni _((void));

static void
aeni()
{
    redirect("aeni");
}


static void aenpr _((void));

static void
aenpr()
{
    redirect("aenpr");
}


static void aenrls _((void));

static void
aenrls()
{
    redirect("aenrls");
}


static void aenrv _((void));

static void
aenrv()
{
    redirect("aenrv");
}


static void aent _((void));

static void
aent()
{
    redirect("aent");
}


static void aentu _((void));

static void
aentu()
{
    redirect("aentu");
}


static void aepa _((void));

static void
aepa()
{
    redirect("aepa");
}


static void aenpa _((void));

static void
aenpa()
{
    redirect("aenpa");
}


static void aerpa _((void));

static void
aerpa()
{
    redirect("aerpa");
}


static void aera _((void));

static void
aera()
{
    redirect("aera");
}


static void aerd _((void));

static void
aerd()
{
    redirect("aerd");
}


static void aerm _((void));

static void
aerm()
{
    redirect("aerm");
}


static void aermu _((void));

static void
aermu()
{
    redirect("aermu");
}


static void aermpr _((void));

static void
aermpr()
{
    redirect("aermpr");
}


static void aeri _((void));

static void
aeri()
{
    redirect("aeri");
}


static void aerrv _((void));

static void
aerrv()
{
    redirect("aerrv");
}


static void aer _((void));

static void
aer()
{
    redirect("aer");
}


static void aerb _((void));

static void
aerb()
{
    redirect("aerb");
}


static void aerbu _((void));

static void
aerbu()
{
    redirect("aerbu");
}


static void aerfail _((void));

static void
aerfail()
{
    redirect("aerfail");
}


static void aerpass _((void));

static void
aerpass()
{
    redirect("aerpass");
}


static void aerpu _((void));

static void
aerpu()
{
    redirect("aerpu");
}


static void aet _((void));

static void
aet()
{
    redirect("aet");
}


static void aev _((void));

static void
aev()
{
    redirect("aev");
}


static void ael _((void));

static void
ael()
{
    redirect("ael");
}


static void usage _((void));

static void
usage()
{
    result = complete_nil();
}


static complete_ty *completion_get _((command_ty *));

static complete_ty *
completion_get(cmd)
    command_ty     *cmd;
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_build, aeb,},
	{ arglex_token_change_attributes, aeca,},
	{ arglex_token_change_directory, aecd,},
	{ arglex_token_change_owner, aechown,},
	{ arglex_token_clean, aeclean,},
	{ arglex_token_clone, aeclone,},
	{ arglex_token_copy_file, aecp,},
	{ arglex_token_copy_file_undo, aecpu,},
	{ arglex_token_develop_begin, aedb,},
	{ arglex_token_develop_begin_undo, aedbu,},
	{ arglex_token_develop_end, aede,},
	{ arglex_token_develop_end_undo, aedeu,},
	{ arglex_token_difference, aed,},
	{ arglex_token_delta_name, aedn,},
	{ arglex_token_integrate_begin, aeib,},
	{ arglex_token_integrate_begin_undo, aeibu,},
	{ arglex_token_integrate_fail, aeifail,},
	{ arglex_token_integrate_pass, aeipass,},
	{ arglex_token_move_file, aemv,},
	{ arglex_token_move_file_undo, aemvu,},
	{ arglex_token_new_administrator, aena,},
	{ arglex_token_new_branch, aenbr,},
	{ arglex_token_new_branch_undo, aenbru,},
	{ arglex_token_new_change, aenc,},
	{ arglex_token_new_change_undo, aencu,},
	{ arglex_token_new_developer, aend,},
	{ arglex_token_new_file, aenf,},
	{ arglex_token_new_file_undo, aenfu,},
	{ arglex_token_new_integrator, aeni,},
	{ arglex_token_new_project, aenpr,},
	{ arglex_token_new_release, aenrls,},
	{ arglex_token_new_reviewer, aenrv,},
	{ arglex_token_new_test, aent,},
	{ arglex_token_new_test_undo, aentu,},
	{ arglex_token_project_attributes, aepa,},
	{ arglex_token_project_alias_create, aenpa,},
	{ arglex_token_project_alias_remove, aerpa,},
	{ arglex_token_remove_administrator, aera,},
	{ arglex_token_remove_developer, aerd,},
	{ arglex_token_remove_file, aerm,},
	{ arglex_token_remove_file_undo, aermu,},
	{ arglex_token_remove_project, aermpr,},
	{ arglex_token_remove_integrator, aeri,},
	{ arglex_token_remove_reviewer, aerrv,},
	{ arglex_token_report, aer,},
	{ arglex_token_review_begin, aerb,},
	{ arglex_token_review_begin_undo, aerbu,},
	{ arglex_token_review_fail, aerfail,},
	{ arglex_token_review_pass, aerpass,},
	{ arglex_token_review_pass_undo, aerpu,},
	{ arglex_token_test, aet,},

	/*
	 * Then there are the more ambiguous arguments.
	 * These are only considered if none of the above are
	 * present on the command line (help least of all,
	 * because everything has help).
	 */
	{ arglex_token_version, aev, 1,},
	{ arglex_token_list, ael, 1,},
    };

    arglex2_retable(0);
    arglex_dispatch(dispatch, SIZEOF(dispatch), usage);
    return result;
}


static command_vtbl_ty vtbl =
{
    destructor,
    completion_get,
    sizeof(command_ty),
    "aegis",
};


command_ty     *
command_aegis()
{
    return command_new(&vtbl);
}
