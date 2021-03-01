//
//	aegis - project change supervisor
//	Copyright (C) 1992-1995, 1997, 1999, 2002-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h> // for umask prototype

#include <common/env.h>
#include <common/error.h>
#include <common/mem.h>
#include <common/quit.h>
#include <common/str.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/output/pager.h>
#include <libaegis/output/stdout.h>
#include <libaegis/quit/action/pager.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


static quit_action_pager cleanup_action;
static int option_pager_flag = -1;
static output::pointer singleton;


void
option_pager_set(int n, void (*usage)(void))
{
    if (option_pager_flag == 0 && n == 0)
	duplicate_option_by_name(arglex_token_pager_not, usage);
    if (option_pager_flag >= 1 && n != 0)
	duplicate_option_by_name(arglex_token_pager, usage);
    if (option_pager_flag >= 0)
    {
	mutually_exclusive_options
	(
	    arglex_token_pager,
	    arglex_token_pager_not,
	    usage
	);
    }
    option_pager_flag = (n != 0);
}


static int
option_pager_get(void)
{
    if (option_pager_flag < 0)
    {
	user_ty::pointer up = user_ty::create();
	option_pager_flag = up->pager_preference();
    }
    return option_pager_flag;
}


void
output_pager::pipe_open()
{
    sub_context_ty  *scp;
    int		    uid;
    int		    gid;
    int		    um;
    FILE	    *fp;
    int		    fd[2];
    const char      *cmd[4];
    int		    pid_;
    int             errno_old;

    env_set_page();
    fp = 0;
    os_become_orig_query(&uid, &gid, &um);
    if (pipe(fd))
	nfatal("pipe()");
    switch (pid_ = fork())
    {
    case 0:
	undo_cancel();
	while (os_become_active())
	    os_become_undo();
	cmd[0] = "sh";
	cmd[1] = "-c";
	cmd[2] = pager.c_str();
	cmd[3] = 0;
	close(fd[1]);
	close(0);
	if (dup(fd[0]) != 0)
	    fatal_raw("dup was wrong");
	close(fd[0]);
	os_setgid(gid);
	os_setuid(uid);
	umask(um);
	execvp(cmd[0], (char **)cmd);

	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_string(scp, "File_Name", pager);
	fatal_intl(scp, i18n("exec \"$filename\": $errno"));
	// NOTREACHED

    case -1:
	nfatal("fork()");
	break;

    default:
	pid = pid_;
	close(fd[0]);
	fp = fdopen(fd[1], "w");
	if (!fp)
	    nfatal("fdopen");
	vdeeper = (void *)fp;
	break;
    }
}


void
output_pager::cleanup()
{
    trace(("output_pager_cleanup()\n{\n"));
    output::pointer temp;
    temp.swap(singleton);
    trace(("}\n"));
}


void
output_pager::pager_error()
{
    int errno_old = errno;
    sub_context_ty sc;
    sc.errno_setx(errno_old);
    sc.var_set_string("File_Name", pager);
    sc.fatal_intl(i18n("write $filename: $errno"));
    // NOTREACHED
}


output_pager::~output_pager()
{
    trace(("output_pager::destructor(this = %08lX)\n{\n", (long)this));
    assert(vdeeper);

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // write the last of the output
    //
    if (fflush((FILE *)vdeeper))
	pager_error();

    //
    // close the paginator
    //
    fclose((FILE *)vdeeper);
    vdeeper = 0;
    int status = 0;
    os_waitpid(pid, &status);
    pid = 0;
    trace(("}\n"));
}


output_pager::output_pager() :
    vdeeper(0),
    pid(0),
    pager(user_ty::create()->pager_command()),
    bol(true)
{
    os_become_orig();
    pipe_open();
    os_become_undo();
    assert(vdeeper);
}


nstring
output_pager::filename()
    const
{
    return pager;
}


long
output_pager::ftell_inner()
    const
{
    return -1;
}


void
output_pager::write_inner(const void *data, size_t len)
{
    if (fwrite(data, 1, len, (FILE *)vdeeper) == 0)
	pager_error();
    if (len > 0)
	bol = (((const char *)data)[len - 1] == '\n');
}


void
output_pager::flush_inner()
{
    if (fflush((FILE *)vdeeper))
	pager_error();
}


void
output_pager::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_pager::type_name()
    const
{
    return "pager";
}


output::pointer
output_pager::open()
{
    trace(("output_pager_open()\n{\n"));
    if (singleton)
    {
        trace(("}\n"));
        return singleton;
    }

    //
    // If talking to a terminal,
    // send the output through a paginator.
    // If we're not, just use stdout.
    //
    if
    (
	!option_pager_get()
    ||
	option_unformatted_get()
    ||
	os_background()
    ||
	!isatty(0)
    ||
	!isatty(1)
    )
    {
	output::pointer result = output_stdout::create();
        singleton = result;
	trace(("return %08lX;\n}\n", (long)result.get()));
	return result;
    }

    //
    // register the cleanup function in case of fatal errors
    //
    quit_register(cleanup_action);

    //
    // open the paginator
    //
    output::pointer result(new output_pager());

    //
    // We keep track of open paginators,
    // so we can clean up after them if necessary.
    //
    singleton = result;
    trace(("return %08lX;\n}\n", (long)result.get()));
    return result;
}
