//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2008, 2012 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>
#include <common/ac/signal.h>

#include <common/sizeof.h>

#include <aegis/aeb.h>
#include <aegis/aeca.h>
#include <aegis/aecd.h>
#include <aegis/aechown.h>
#include <aegis/aeclean.h>
#include <aegis/aeclone.h>
#include <aegis/aecp.h>
#include <aegis/aecpu.h>
#include <aegis/aed.h>
#include <aegis/aedb.h>
#include <aegis/aedbu.h>
#include <aegis/aede.h>
#include <aegis/aedeu.h>
#include <aegis/aedn.h>
#include <aegis/aefa.h>
#include <aegis/aeib.h>
#include <aegis/aeibu.h>
#include <aegis/aeif.h>
#include <aegis/aeip.h>
#include <aegis/ael.h>
#include <aegis/aemt.h>
#include <aegis/aemtu.h>
#include <aegis/aemv.h>
#include <aegis/aemvu.h>
#include <aegis/aena.h>
#include <aegis/aenbr.h>
#include <aegis/aenbru.h>
#include <aegis/aenc.h>
#include <aegis/aencu.h>
#include <aegis/aend.h>
#include <aegis/aenf.h>
#include <aegis/aenfu.h>
#include <aegis/aeni.h>
#include <aegis/aenpa.h>
#include <aegis/aenpr.h>
#include <aegis/aenrls.h>
#include <aegis/aenrv.h>
#include <aegis/aent.h>
#include <aegis/aentu.h>
#include <aegis/aepa.h>
#include <aegis/aer.h>
#include <aegis/aera.h>
#include <aegis/aerb.h>
#include <aegis/aerbu.h>
#include <aegis/aerd.h>
#include <aegis/aerf.h>
#include <aegis/aeri.h>
#include <aegis/aerm.h>
#include <aegis/aermpr.h>
#include <aegis/aermu.h>
#include <aegis/aerp.h>
#include <aegis/aerpa.h>
#include <aegis/aerpu.h>
#include <aegis/aerrv.h>
#include <aegis/aet.h>
#include <libaegis/arglex2.h>
#include <aegis/configured.h>
#include <common/env.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/version.h>


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s <function> [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
main_help(void)
{
    trace(("main_help()\n{\n"));
    help((char *)0, usage);
    trace(("}\n"));
}


int
main(int argc, char **argv)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_build, build, 0 },
        { arglex_token_change_attributes, change_attributes, 0 },
        { arglex_token_change_directory, change_directory, 0 },
        { arglex_token_change_owner, change_owner, 0 },
        { arglex_token_clean, clean, 0 },
        { arglex_token_clone, aeclone, 0 },
        { arglex_token_configured, configured, 0 },
        { arglex_token_copy_file, copy_file, 0 },
        { arglex_token_copy_file_undo, copy_file_undo, 0 },
        { arglex_token_develop_begin, develop_begin, 0 },
        { arglex_token_develop_begin_undo, develop_begin_undo, 0 },
        { arglex_token_develop_end, develop_end, 0 },
        { arglex_token_develop_end_undo, develop_end_undo, 0 },
        { arglex_token_difference, difference, 0 },
        { arglex_token_delta_name, delta_name_assignment, 0 },
        { arglex_token_file_attributes, file_attributes, 0 },
        { arglex_token_integrate_begin, integrate_begin, 0 },
        { arglex_token_integrate_begin_undo, integrate_begin_undo, 0 },
        { arglex_token_integrate_fail, integrate_fail, 0 },
        { arglex_token_integrate_pass, integrate_pass, 0 },
        { arglex_token_move_file, move_file, 0 },
        { arglex_token_move_file_undo, move_file_undo, 0 },
        { arglex_token_new_administrator, new_administrator, 0 },
        { arglex_token_new_branch, new_branch, 0 },
        { arglex_token_new_branch_undo, new_branch_undo, 0 },
        { arglex_token_new_change, new_change, 0 },
        { arglex_token_new_change_undo, new_change_undo, 0 },
        { arglex_token_new_developer, new_developer, 0 },
        { arglex_token_make_transparent, make_transparent, 0 },
        { arglex_token_make_transparent_undo, make_transparent_undo, 0 },
        { arglex_token_new_file, new_file, 0 },
        { arglex_token_new_file_undo, new_file_undo, 0 },
        { arglex_token_new_integrator, new_integrator, 0 },
        { arglex_token_new_project, new_project, 0 },
        { arglex_token_new_release, new_release, 0 },
        { arglex_token_new_reviewer, new_reviewer, 0 },
        { arglex_token_new_test, new_test, 0 },
        { arglex_token_new_test_undo, new_test_undo, 0 },
        { arglex_token_project_attributes, project_attributes, 0 },
        { arglex_token_project_alias_create, project_alias_create, 0 },
        { arglex_token_project_alias_remove, project_alias_remove, 0 },
        { arglex_token_remove_administrator, remove_administrator, 0 },
        { arglex_token_remove_developer, remove_developer, 0 },
        { arglex_token_remove_file, remove_file, 0 },
        { arglex_token_remove_file_undo, remove_file_undo, 0 },
        { arglex_token_remove_project, remove_project, 0 },
        { arglex_token_remove_integrator, remove_integrator, 0 },
        { arglex_token_remove_reviewer, remove_reviewer, 0 },
        { arglex_token_report, report, 0 },
        { arglex_token_review_begin, review_begin, 0 },
        { arglex_token_review_begin_undo, review_begin_undo, 0 },
        { arglex_token_review_fail, review_fail, 0 },
        { arglex_token_review_pass, review_pass, 0 },
        { arglex_token_review_pass_undo, review_pass_undo, 0 },
        { arglex_token_test, test, 0 },

        //
        // Then there are the more ambiguous arguments.
        // These are only considered if none of the above are
        // present on the command line (help least of all,
        // because everything has help).
        //
        { arglex_token_version, version, 1 },
        { arglex_token_list, list, 1 },
        { arglex_token_help, main_help, 2 },
    };

    //
    // Some versions of cron(8) set SIGCHLD to SIG_IGN.  This is
    // kinda dumb, because it breaks assumptions made in libc (like
    // pclose, for instance).  It also blows away most of Cook's
    // process handling.  We explicitly set the SIGCHLD signal
    // handling to SIG_DFL to make sure this signal does what we
    // expect no matter how we are invoked.
    //
#ifdef SIGCHLD
    signal(SIGCHLD, SIG_DFL);
#else
    signal(SIGCLD, SIG_DFL);
#endif

    resource_limits_init();
    os_become_init();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();
    quit_register(log_quitter);
    quit_register(undo_quitter);
    os_interrupt_register();

    arglex_dispatch(dispatch, SIZEOF(dispatch), usage);

    trace(("quit(0);\n"));
    quit(0);
    trace(("return 0;\n"));
    return 0;
}


// vim: set ts=8 sw=4 et :
