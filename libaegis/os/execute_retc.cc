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

#include <common/ac/stdio.h>
#include <common/ac/errno.h>
#include <common/ac/signal.h>
#include <common/ac/stddef.h>
#include <common/ac/unistd.h>

#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/env.h>
#include <common/error.h>
#include <common/trace.h>
#include <libaegis/getgr_cache.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/os/interrupt.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>


#define MAX_CMD_RPT 36


static void
who_and_where(int uid, int gid, const nstring &dir)
{
    trace(("mark\n"));
    if (!option_verbose_get())
	return;
    static int last_uid;
    static int last_gid;
    static nstring last_dir;
    if (last_dir.empty())
    {
	os_become_orig_query(&last_uid, &last_gid, (int *)0);
	last_dir = nstring(os_curdir());
    }
    nstring rdir = os_pathname(dir, true);
    if (last_dir != rdir)
    {
	sub_context_ty sc;
	last_dir = rdir;
	sc.var_set_string("File_Name", dir);
	sc.error_intl(i18n("cd $filename"));
    }

    if (last_uid != uid || last_gid != gid)
    {
	sub_context_ty sc;

	last_uid = uid;
	struct passwd *pw = getpwuid_cached(uid);
	if (pw)
	    sc.var_set_format("Name1", "\"%s\"", pw->pw_name);
	else
	    sc.var_set_long("Name1", uid);

	last_gid = gid;
	struct group *gr = getgrgid_cached(gid);
	if (gr)
	    sc.var_set_format("Name2", "\"%s\"", gr->gr_name);
	else
	    sc.var_set_long("Name2", gid);

	sc.error_intl(i18n("user $name1, group $name2"));
    }
}


int
os_execute_retcode(string_ty *cmd, int flags, string_ty *dir)
{
    trace(("mark\n"));
    return os_execute_retcode(nstring(cmd), flags, nstring(dir));
}


int
os_execute_retcode(const nstring &cmd, int flags, const nstring &dir)
{
    trace(("os_execute_retcode()\n{\n"));
    int result = 0;
    os_become_must_be_active();
    int uid = 0, gid = 0, um = 0;
    os_become_query(&uid, &gid, &um);
    who_and_where(uid, gid, dir);
    nstring cmd2 = cmd;
    if (!(flags & OS_EXEC_FLAG_SILENT) && cmd.size() > MAX_CMD_RPT)
	cmd2 = nstring::format("%.*s...", MAX_CMD_RPT - 3, cmd.c_str());

    //
    // Remember the user name, so we can set the environment variable.
    // We do this before the fork, so it will be cached for next time.
    //
    (void)getpwuid_cached(uid);

    //
    // fork to get a process to do the command
    //
    trace(("mark\n"));
    int child = fork();
    switch (child)
    {
    case -1:
	nfatal("fork");

    default:
        {
            //
            // The parent process waits
            //
            RETSIGTYPE (*hold)(int) = signal(SIGINT, SIG_IGN);
            if (hold != SIG_IGN)
                signal(SIGINT, os_interrupt);
            trace(("parent\n"));
            fflush(stderr);
            result = os_waitpid_status(child, cmd2.c_str());
            trace(("result = 0x%04X\n", result));
        }
	break;

    case 0:
	//
	// become the user for real
	//
        trace(("child\n"));
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
        trace(("child\n"));
	env_set_page();

        //
        // Set the user name to match the UID
        // (we cached it before the fork)
        //
        {
            struct passwd *pw = getpwuid_cached(uid);
            if (pw)
            {
                env_set("USER", pw->pw_name);
                env_set("LOGIN", pw->pw_name);
                env_set("USERNAME", pw->pw_name);
                env_set("LOGNAME", pw->pw_name);
                env_set("HOME", pw->pw_dir);
            }
        }

	//
	// Redirect stdin from a broken pipe.
	// (Don't redirect stdin if not logging, for manual tests.)
	//
	if (!(flags & OS_EXEC_FLAG_INPUT))
	{
	    int pfd[2];
	    if (pipe(pfd))
		nfatal("pipe");
	    if (close(0))
		nfatal("close stdin");
	    int n = dup(pfd[0]);
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
        trace(("child\n"));
	if (!(flags & OS_EXEC_FLAG_SILENT))
	{
            sub_context_ty sc;
	    sc.var_set_string("MeSsaGe", cmd);
	    sc.error_intl("$message");
	}

	//
	// invoke the command through sh(1)
	//
        trace(("child\n"));
	const char *shell = os_shell();
	execl(shell, shell, "-ec", cmd.c_str(), (char *)0);
	int errno_old = errno;
        sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_charstar("File_Name", shell);
	sc.fatal_intl(i18n("exec \"$filename\": $errno"));
	// NOTREACHED
    }
    trace(("parent\n"));
    if (result && (flags & OS_EXEC_FLAG_ERROK))
    {
	sub_context_ty sc;
	sc.var_set_string("Command", cmd);
	sc.var_set_long("Number", result);
	sc.error_intl
	(
	    i18n("warning: command \"$command\" exit status $number")
	);
	result = 0;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}
