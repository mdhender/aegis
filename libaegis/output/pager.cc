//
//	aegis - project change supervisor
//	Copyright (C) 1992-1995, 1997, 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to pipe output through paginator
//

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <ac/sys/types.h>
#include <sys/stat.h> // for umask prototype

#include <arglex2.h>
#include <env.h>
#include <error.h>
#include <help.h>
#include <option.h>
#include <os.h>
#include <output.h>
#include <output/pager.h>
#include <output/stdout.h>
#include <quit.h>
#include <quit/action/pager.h>
#include <str.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static quit_action_pager cleanup;
static int option_pager_flag = -1;
static output_ty *singleton;


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
	user_ty *up = user_executing(0);
	option_pager_flag = user_pager_preference(up);
	user_free(up);
    }
    return option_pager_flag;
}


void
output_pager_ty::pipe_open()
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
output_pager_cleanup()
{
    trace(("output_pager::cleanup()\n{\n"));
    if (singleton)
    {
	output_ty	*p;

	p = singleton;
	singleton = 0;
	delete p;
    }
    trace(("}\n"));
}


void
output_pager_ty::pager_error()
{
    int errno_old = errno;
    sub_context_ty sc;
    sc.errno_setx(errno_old);
    sc.var_set_string("File_Name", pager);
    sc.fatal_intl(i18n("write $filename: $errno"));
    // NOTREACHED
}


output_pager_ty::~output_pager_ty()
{
    trace(("output_pager::destructor(this = %08lX)\n{\n", (long)this));
    assert(vdeeper);

    //
    // Make sure all buffered data has been passed to our write_inner
    // method.
    //
    flush();

    //
    // nuke the singelton
    //
    assert(singleton == this);
    singleton = 0;

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


output_pager_ty::output_pager_ty() :
    vdeeper(0),
    pid(0),
    pager(str_copy(user_pager_command(0))),
    bol(true)
{
    os_become_orig();
    pipe_open();
    os_become_undo();
    assert(vdeeper);

    //
    // We keep track of open paginators,
    // so we can clean up after them if necessary.
    //
    singleton = this;
}


string_ty *
output_pager_ty::filename()
    const
{
    return pager.get_ref();
}


long
output_pager_ty::ftell_inner()
    const
{
    return -1;
}


void
output_pager_ty::write_inner(const void *data, size_t len)
{
    if (fwrite(data, 1, len, (FILE *)vdeeper) == 0)
	pager_error();
    if (len > 0)
	bol = (((const char *)data)[len - 1] == '\n');
}


void
output_pager_ty::flush_inner()
{
    if (fflush((FILE *)vdeeper))
	pager_error();
}


void
output_pager_ty::end_of_line_inner()
{
    if (!bol)
	fputc('\n');
}


const char *
output_pager_ty::type_name()
    const
{
    return "pager";
}


output_ty *
output_pager_open()
{
    trace(("output_pager_open()\n{\n"));
    assert(!singleton);

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
    ||
	singleton
    )
    {
	output_ty *result = new output_stdout();
	trace(("return %08lX;\n}\n", (long)result));
	return result;
    }

    //
    // register the cleanup function in case of fatal errors
    //
    quit_register(cleanup);

    //
    // open the paginator
    //
    output_ty *result = new output_pager_ty();
    trace(("return %08lX;\n}\n", (long)result));
    return result;
}
