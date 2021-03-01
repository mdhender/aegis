//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2004 Peter Miller;
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
// MANIFEST: the operating system start-up point
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <ac/signal.h>

#include <aeb.h>
#include <aeca.h>
#include <aecd.h>
#include <aechown.h>
#include <aeclean.h>
#include <aeclone.h>
#include <aecp.h>
#include <aecpu.h>
#include <aed.h>
#include <aedb.h>
#include <aedbu.h>
#include <aede.h>
#include <aedeu.h>
#include <aedn.h>
#include <aefa.h>
#include <aeib.h>
#include <aeibu.h>
#include <aeif.h>
#include <aeip.h>
#include <ael.h>
#include <aemt.h>
#include <aemtu.h>
#include <aemv.h>
#include <aemvu.h>
#include <aena.h>
#include <aenbr.h>
#include <aenbru.h>
#include <aenc.h>
#include <aencu.h>
#include <aend.h>
#include <aenf.h>
#include <aenfu.h>
#include <aeni.h>
#include <aenpa.h>
#include <aenpr.h>
#include <aenrls.h>
#include <aenrv.h>
#include <aent.h>
#include <aentu.h>
#include <aepa.h>
#include <aer.h>
#include <aera.h>
#include <aerb.h>
#include <aerbu.h>
#include <aerd.h>
#include <aerf.h>
#include <aeri.h>
#include <aerm.h>
#include <aermpr.h>
#include <aermu.h>
#include <aerp.h>
#include <aerpa.h>
#include <aerpu.h>
#include <aerrv.h>
#include <aet.h>
#include <arglex2.h>
#include <configured.h>
#include <env.h>
#include <help.h>
#include <language.h>
#include <log.h>
#include <os.h>
#include <progname.h>
#include <quit.h>
#include <r250.h>
#include <trace.h>
#include <undo.h>
#include <version.h>


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
	{ arglex_token_build,			build,			},
	{ arglex_token_change_attributes,	change_attributes,	},
	{ arglex_token_change_directory,	change_directory,	},
	{ arglex_token_change_owner,		change_owner,		},
	{ arglex_token_clean,			clean,			},
	{ arglex_token_clone,			aeclone,		},
	{ arglex_token_configured,		configured,		},
	{ arglex_token_copy_file,		copy_file,		},
	{ arglex_token_copy_file_undo,		copy_file_undo,		},
	{ arglex_token_develop_begin,		develop_begin,		},
	{ arglex_token_develop_begin_undo,	develop_begin_undo,	},
	{ arglex_token_develop_end,		develop_end,		},
	{ arglex_token_develop_end_undo,	develop_end_undo,	},
	{ arglex_token_difference,		difference,		},
	{ arglex_token_delta_name,		delta_name_assignment	},
	{ arglex_token_file_attributes,		file_attributes,	},
	{ arglex_token_integrate_begin,		integrate_begin,	},
	{ arglex_token_integrate_begin_undo,	integrate_begin_undo,	},
	{ arglex_token_integrate_fail,		integrate_fail,		},
	{ arglex_token_integrate_pass,		integrate_pass,		},
	{ arglex_token_move_file,		move_file,		},
	{ arglex_token_move_file_undo,		move_file_undo,		},
	{ arglex_token_new_administrator,	new_administrator,	},
	{ arglex_token_new_branch,		new_branch,		},
	{ arglex_token_new_branch_undo,		new_branch_undo,	},
	{ arglex_token_new_change,		new_change,		},
	{ arglex_token_new_change_undo,		new_change_undo,	},
	{ arglex_token_new_developer,		new_developer,		},
	{ arglex_token_make_transparent,	make_transparent,	},
	{ arglex_token_make_transparent_undo,  make_transparent_undo,	},
	{ arglex_token_new_file,		new_file,		},
	{ arglex_token_new_file_undo,		new_file_undo,		},
	{ arglex_token_new_integrator,		new_integrator,		},
	{ arglex_token_new_project,		new_project,		},
	{ arglex_token_new_release,		new_release,		},
	{ arglex_token_new_reviewer,		new_reviewer,		},
	{ arglex_token_new_test,		new_test,		},
	{ arglex_token_new_test_undo,		new_test_undo,		},
	{ arglex_token_project_attributes,	project_attributes,	},
	{ arglex_token_project_alias_create,	project_alias_create,	},
	{ arglex_token_project_alias_remove,	project_alias_remove,	},
	{ arglex_token_remove_administrator,	remove_administrator,	},
	{ arglex_token_remove_developer,	remove_developer,	},
	{ arglex_token_remove_file,		remove_file,		},
	{ arglex_token_remove_file_undo,	remove_file_undo,	},
	{ arglex_token_remove_project,		remove_project,		},
	{ arglex_token_remove_integrator,	remove_integrator,	},
	{ arglex_token_remove_reviewer,		remove_reviewer,	},
	{ arglex_token_report,			report,			},
	{ arglex_token_review_begin,		review_begin,		},
	{ arglex_token_review_begin_undo,	review_begin_undo,	},
	{ arglex_token_review_fail,		review_fail,		},
	{ arglex_token_review_pass,		review_pass,		},
	{ arglex_token_review_pass_undo,	review_pass_undo,	},
	{ arglex_token_test,			test,			},

	//
	// Then there are the more ambiguous arguments.
	// These are only considered if none of the above are
	// present on the command line (help least of all,
	// because everything has help).
	//
	{ arglex_token_version,			version,	1,	},
	{ arglex_token_list,			list,		1,	},
	{ arglex_token_help,			main_help,	2,	},
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

    r250_init();
    os_become_init();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();
    quit_register(log_quitter);
    quit_register(undo_quitter);
    os_interrupt_register();

    arglex_dispatch(dispatch, SIZEOF(dispatch), usage);

    quit(0);
    return 0;
}
