//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <libaegis/arglex2.h>
#include <aecomplete/command/aegis.h>
#include <aecomplete/command/private.h>
#include <aecomplete/complete/nil.h>
#include <aecomplete/complete/project/name.h>
#include <aecomplete/complete/project/name.h>


static complete_ty *result;


static void
destructor(command_ty *)
{
}


static void
redirect(const char *name)
{
    command_ty     *cmd;
    string_ty      *s;

    s = str_from_c(name);
    cmd = command_find(s);
    str_free(s);
    result = command_completion_get(cmd);
}


static void
aeb(void)
{
    redirect("aeb");
}


static void
aeca(void)
{
    redirect("aeca");
}


static void
aecd(void)
{
    redirect("aecd");
}


static void
aechown(void)
{
    redirect("aechown");
}


static void
aeclean(void)
{
    redirect("aeclean");
}


static void
aeclone(void)
{
    redirect("aeclone");
}


static void
aecp(void)
{
    redirect("aecp");
}


static void
aecpu(void)
{
    redirect("aecpu");
}


static void
aedb(void)
{
    redirect("aedb");
}


static void
aedbu(void)
{
    redirect("aedbu");
}


static void
aede(void)
{
    redirect("aede");
}


static void
aedeu(void)
{
    redirect("aedeu");
}


static void
aed(void)
{
    redirect("aed");
}


static void
aedn(void)
{
    redirect("aedn");
}


static void
aeib(void)
{
    redirect("aeib");
}


static void
aeibu(void)
{
    redirect("aeibu");
}


static void
aeifail(void)
{
    redirect("aeifail");
}


static void
aeipass(void)
{
    redirect("aeipass");
}


static void
aemv(void)
{
    redirect("aemv");
}


static void
aemvu(void)
{
    redirect("aemvu");
}


static void
aena(void)
{
    redirect("aena");
}


static void
aenbr(void)
{
    redirect("aenbr");
}


static void
aenbru(void)
{
    redirect("aenbru");
}


static void
aenc(void)
{
    redirect("aenc");
}


static void
aencu(void)
{
    redirect("aencu");
}


static void
aend(void)
{
    redirect("aend");
}


static void
aenf(void)
{
    redirect("aenf");
}


static void
aenfu(void)
{
    redirect("aenfu");
}


static void
aeni(void)
{
    redirect("aeni");
}


static void
aenpr(void)
{
    redirect("aenpr");
}


static void
aenrls(void)
{
    redirect("aenrls");
}


static void
aenrv(void)
{
    redirect("aenrv");
}


static void
aent(void)
{
    redirect("aent");
}


static void
aentu(void)
{
    redirect("aentu");
}


static void
aepa(void)
{
    redirect("aepa");
}


static void
aenpa(void)
{
    redirect("aenpa");
}


static void
aerpa(void)
{
    redirect("aerpa");
}


static void
aera(void)
{
    redirect("aera");
}


static void
aerd(void)
{
    redirect("aerd");
}


static void
aerm(void)
{
    redirect("aerm");
}


static void
aermu(void)
{
    redirect("aermu");
}


static void
aermpr(void)
{
    redirect("aermpr");
}


static void
aeri(void)
{
    redirect("aeri");
}


static void
aerrv(void)
{
    redirect("aerrv");
}


static void
aer(void)
{
    redirect("aer");
}


static void
aerb(void)
{
    redirect("aerb");
}


static void
aerbu(void)
{
    redirect("aerbu");
}


static void
aerfail(void)
{
    redirect("aerfail");
}


static void
aerpass(void)
{
    redirect("aerpass");
}


static void
aerpu(void)
{
    redirect("aerpu");
}


static void
aet(void)
{
    redirect("aet");
}


static void
aev(void)
{
    redirect("aev");
}


static void
ael(void)
{
    redirect("ael");
}


static void
usage(void)
{
    result = complete_nil();
}


static complete_ty *
completion_get(command_ty *)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_build, aeb, 0 },
	{ arglex_token_change_attributes, aeca, 0 },
	{ arglex_token_change_directory, aecd, 0 },
	{ arglex_token_change_owner, aechown, 0 },
	{ arglex_token_clean, aeclean, 0 },
	{ arglex_token_clone, aeclone, 0 },
	{ arglex_token_copy_file, aecp, 0 },
	{ arglex_token_copy_file_undo, aecpu, 0 },
	{ arglex_token_develop_begin, aedb, 0 },
	{ arglex_token_develop_begin_undo, aedbu, 0 },
	{ arglex_token_develop_end, aede, 0 },
	{ arglex_token_develop_end_undo, aedeu, 0 },
	{ arglex_token_difference, aed, 0 },
	{ arglex_token_delta_name, aedn, 0 },
	{ arglex_token_integrate_begin, aeib, 0 },
	{ arglex_token_integrate_begin_undo, aeibu, 0 },
	{ arglex_token_integrate_fail, aeifail, 0 },
	{ arglex_token_integrate_pass, aeipass, 0 },
	{ arglex_token_move_file, aemv, 0 },
	{ arglex_token_move_file_undo, aemvu, 0 },
	{ arglex_token_new_administrator, aena, 0 },
	{ arglex_token_new_branch, aenbr, 0 },
	{ arglex_token_new_branch_undo, aenbru, 0 },
	{ arglex_token_new_change, aenc, 0 },
	{ arglex_token_new_change_undo, aencu, 0 },
	{ arglex_token_new_developer, aend, 0 },
	{ arglex_token_new_file, aenf, 0 },
	{ arglex_token_new_file_undo, aenfu, 0 },
	{ arglex_token_new_integrator, aeni, 0 },
	{ arglex_token_new_project, aenpr, 0 },
	{ arglex_token_new_release, aenrls, 0 },
	{ arglex_token_new_reviewer, aenrv, 0 },
	{ arglex_token_new_test, aent, 0 },
	{ arglex_token_new_test_undo, aentu, 0 },
	{ arglex_token_project_attributes, aepa, 0 },
	{ arglex_token_project_alias_create, aenpa, 0 },
	{ arglex_token_project_alias_remove, aerpa, 0 },
	{ arglex_token_remove_administrator, aera, 0 },
	{ arglex_token_remove_developer, aerd, 0 },
	{ arglex_token_remove_file, aerm, 0 },
	{ arglex_token_remove_file_undo, aermu, 0 },
	{ arglex_token_remove_project, aermpr, 0 },
	{ arglex_token_remove_integrator, aeri, 0 },
	{ arglex_token_remove_reviewer, aerrv, 0 },
	{ arglex_token_report, aer, 0 },
	{ arglex_token_review_begin, aerb, 0 },
	{ arglex_token_review_begin_undo, aerbu, 0 },
	{ arglex_token_review_fail, aerfail, 0 },
	{ arglex_token_review_pass, aerpass, 0 },
	{ arglex_token_review_pass_undo, aerpu, 0 },
	{ arglex_token_test, aet, 0 },

	//
	// Then there are the more ambiguous arguments.
	// These are only considered if none of the above are
	// present on the command line (help least of all,
	// because everything has help).
	//
	{ arglex_token_version, aev, 1 },
	{ arglex_token_list, ael, 1 },
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
