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
// MANIFEST: functions to manipulate execute_retcs
//

#include <ac/errno.h>
#include <ac/grp.h>
#include <ac/pwd.h>
#include <ac/signal.h>
#include <ac/stddef.h>
#include <ac/unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <env.h>
#include <error.h>
#include <option.h>
#include <os.h>
#include <os/interrupt.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>


#define MAX_CMD_RPT 36


static void
who_and_where(int uid, int gid, string_ty *dir)
{
    static int      last_uid;
    static int      last_gid;
    static string_ty *last_dir;
    string_ty       *rdir;

    if (!option_verbose_get())
	return;
    if (!last_dir)
    {
	os_become_orig_query(&last_uid, &last_gid, (int *)0);
	last_dir = os_curdir();
    }
    rdir = os_pathname(dir, 1);
    if (!str_equal(last_dir, rdir))
    {
	sub_context_ty  *scp;

	str_free(last_dir);
	last_dir = rdir;
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", dir);
	error_intl(scp, i18n("cd $filename"));
	sub_context_delete(scp);
    }
    else
	str_free(rdir);

    if (last_uid != uid || last_gid != gid)
    {
	sub_context_ty  *scp;
	struct passwd   *pw;
	struct group    *gr;

	scp = sub_context_new();
	last_uid = uid;
	pw = getpwuid(uid);
	if (pw)
	    sub_var_set_format(scp, "Name1", "\"%s\"", pw->pw_name);
	else
	    sub_var_set_long(scp, "Name1", uid);

	last_gid = gid;
	gr = getgrgid(gid);
	if (gr)
	    sub_var_set_format(scp, "Name2", "\"%s\"", gr->gr_name);
	else
	    sub_var_set_long(scp, "Name2", gid);
	error_intl(scp, i18n("user $name1, group $name2"));
	sub_context_delete(scp);
    }
}


int
os_execute_retcode(string_ty *cmd, int flags, string_ty *dir)
{
    sub_context_ty  *scp;
    int             uid;
    int             gid;
    int             um;
    int             child;
    int             result = 0;
    RETSIGTYPE      (*hold)(int);
    string_ty       *cmd2;
    const char      *shell;
    int             errno_old;

    trace(("os_execute_retcode()\n{\n"));
    os_become_must_be_active();
    os_become_query(&uid, &gid, &um);
    who_and_where(uid, gid, dir);
    if (!(flags & OS_EXEC_FLAG_SILENT) && cmd->str_length > MAX_CMD_RPT)
	cmd2 = str_format("%.*s...", MAX_CMD_RPT - 3, cmd->str_text);
    else
	cmd2 = str_copy(cmd);

    //
    // fork to get a process to do the command
    //
    switch (child = fork())
    {
    case -1:
	nfatal("fork");

    default:
	//
	// The parent process waits
	//
	hold = signal(SIGINT, SIG_IGN);
	if (hold != SIG_IGN)
	    signal(SIGINT, os_interrupt);
	result = os_waitpid_status(child, cmd2->str_text);
	break;

    case 0:
	//
	// become the user for real
	//
	while (os_become_active())
	    os_become_undo();
	undo_cancel();
	os_setgid(gid);
	os_setuid(uid);
	umask(um);

	//
	// change directory to the appropriate directory.
	//
	if (dir)
	    os_chdir(dir);

	//
	// We are about to break stdin, one of the file descriptors
	// used by programs to determine the terminal size.  Set the
	// appropriate environment variables to that the sub-program
	// agrees with Aegis.
	//
	env_set_page();

	//
	// Redirect stdin from a broken pipe.
	// (Don't redirect stdin if not logging, for manual tests.)
	//
	if (!(flags & OS_EXEC_FLAG_INPUT))
	{
	    int             pfd[2];
	    int             n;

	    if (pipe(pfd))
		nfatal("pipe");
	    if (close(0))
		nfatal("close stdin");
	    n = dup(pfd[0]);
	    if (n < 0)
		nfatal("dup");
	    if (n != 0)
		fatal_raw("dup gave %d, not 0 (bug)", n);
	    if (close(pfd[0]) || close(pfd[1]))
		nfatal("close pipe ends");
	}

	//
	// let the log file (user) know what we did
	//
	if (!(flags & OS_EXEC_FLAG_SILENT))
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Message", cmd);
	    error_intl(scp, "$message");
	    sub_context_delete(scp);
	}

	//
	// invoke the command through sh(1)
	//
	shell = os_shell();
	execl(shell, shell, "-ec", cmd->str_text, (char *)0);
	errno_old = errno;
	scp = sub_context_new();
	sub_errno_setx(scp, errno_old);
	sub_var_set_charstar(scp, "File_Name", shell);
	fatal_intl(scp, i18n("exec \"$filename\": $errno"));
	// NOTREACHED
    }
    if (result && (flags & OS_EXEC_FLAG_ERROK))
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "Command", cmd);
	sub_var_set_long(scp, "Number", result);
	error_intl
	(
	    scp,
	    i18n("warning: command \"$command\" exit status $number")
	);
	sub_context_delete(scp);
	result = 0;
    }
    str_free(cmd2);
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
