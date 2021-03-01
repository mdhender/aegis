//
//	aegis - project change supervisor
//	Copyright (C) 1991-1997, 1999, 2002-2008 Peter Miller
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

#include <common/ac/stddef.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/signal.h>

#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>	// for umask prototype!
#include <common/ac/unistd.h>
#include <common/ac/fcntl.h>

#include <common/env.h>
#include <common/error.h>
#include <libaegis/lock.h>	// for lock_release_child
#include <libaegis/log.h>
#include <common/now.h>
#include <libaegis/os.h>
#include <libaegis/quit/action/log.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


quit_action_log log_quitter;
static int      pid;


static void
tee_stdout(user_ty::pointer up, char *filename, int also_to_tty,
    int append_to_file)
{
    int             fd[2];
    int             uid;
    int             gid;
    int             um;
    const char      *argv[4];
    int             argc;

    trace(("tee_stdout(up = %08lX, filename = \"%s\", also_to_tty = %d, "
	"append_to_file = %d)\n{\n", (long)up.get(), filename, also_to_tty,
	append_to_file));

    //
    // We are about to mangle stdout, which is one of the file descriptors
    // that programs typically use to determine the width of the
    // terminal.  So we need to set LINES and COLS before we do, so
    // that our sub-commands will think the terminal is the same width
    // that Aegis does.
    //
    env_set_page();

    //
    // list both to a file and to the terminal
    //
    uid = up->get_uid();
    gid = up->get_gid();
    um = up->umask_get();
    if (pipe(fd))
	nfatal("pipe()");
    switch (pid = fork())
    {
    case 0:
	//
	// child
	//
	undo_cancel();
	lock_release_child();
	while (os_become_active())
	    os_become_undo();
	close(fd[1]);
	close(0);
	if (dup(fd[0]) != 0)
	    fatal_raw("dup was wrong");
	close(fd[0]);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	os_setgid(gid);
	os_setuid(uid);
	umask(um);

	if (!also_to_tty)
	{
	    //
	    // Some systems can't write to a user's file
	    // when euid=0 over NFS.  The permissions are
	    // supposed to only apply when the file is
	    // opened, and subsequent writes are not
	    // affected.  Sigh.  Ever seen "Permission
	    // denied" from a write() call?  Eek!
	    //
	    // For systems with no functioning seteuid
	    // call, this is essential, even if NFS writes
	    // behave as they should.
	    //
	    // So: we always open a pipe, and simply run it
	    // through "tee" with the output redirected to
	    // the bit bucket.
	    //
	    close(1);
	    if (1 != open("/dev/null", O_WRONLY, 0666))
		nfatal("open /dev/null");
	}

	//
	// build the tee command
	//
	argc = 0;
	argv[argc++] = "tee";
	if (append_to_file)
	    argv[argc++] = "-a";
	argv[argc++] = filename;
	argv[argc] = 0;

	//
	// try to exec it
	//
	// (The cast is because many operating systems have a stupid
	// prototype, in turn probably because gcc whines incorrectly when
	// you have the correct prototype.)
	//
	execvp(argv[0], (char **)argv);
	nfatal("exec \"%s\"", argv[0]);

    case -1:
	nfatal("fork()");

    default:
	//
	// parent:
	// redirect stdout to go through the pipe
	//
	close(fd[0]);
	close(1);
	if (dup(fd[1]) != 1)
	    fatal_raw("dup was wrong");
	close(fd[1]);
	break;
    }
    trace(("}\n"));
}


static log_style_ty
pref_to_style(uconf_log_file_preference_ty dflt)
{
    user_ty::pointer up = user_ty::create();
    switch (up->log_file_preference(dflt))
    {
    case uconf_log_file_preference_never:
	return log_style_none;

    case uconf_log_file_preference_append:
	return log_style_append;

    case uconf_log_file_preference_snuggle:
	return log_style_snuggle;

    case uconf_log_file_preference_replace:
	return log_style_create;
    }
    assert(0);
    return log_style_append;
}


//
//  NAME
//	log_open - start logging
//
//  SYNOPSIS
//	void log_open(string_ty *logfile, user_ty::pointer up);
//
//  DESCRIPTION
//	The log_open function is used to start sending stdout
//	and stderr to a longfile.  If necessary it creates the log
//	file before returning.
//

void
log_open(string_ty *filename, user_ty::pointer up, log_style_ty style)
{
    static int      already_done;
    int             bg;
    int             append_to_file;
    int             exists;

    if (already_done)
	return;
    assert(filename);
    trace(("log_open(s = \"%s\")\n{\n", filename->str_text));
    already_done = 1;

    //
    // check the user defaults
    //
    switch (style)
    {
    case log_style_none_default:
	style = pref_to_style(uconf_log_file_preference_never);
	break;

    case log_style_create_default:
	style = pref_to_style(uconf_log_file_preference_replace);
	break;

    case log_style_append_default:
	style = pref_to_style(uconf_log_file_preference_append);
	break;

    case log_style_snuggle_default:
	style = pref_to_style(uconf_log_file_preference_snuggle);
	break;

    default:
	break;
    }
    if (style == log_style_none)
    {
	trace(("}\n"));
	return;
    }

    //
    // if the logfile exists, unlink it first
    // (so that baseline linked to int dir works correctly)
    //
    append_to_file = (style == log_style_append);
    up->become_begin();
    exists = os_exists(filename);
    if (style == log_style_snuggle && exists)
    {
	time_t          log_old;
	time_t          log_new;

	os_mtime_range(filename, &log_old, &log_new);
	if (now() - log_new < 30)
	    append_to_file = 1;
    }
    if (!append_to_file && exists)
	os_unlink(filename);
    if (append_to_file && !exists)
	append_to_file = 0;
    up->become_end();

    //
    // If we are in the background,
    // don't send the output to the terminal.
    //
    bg = os_background();
    tee_stdout(up, filename->str_text, !bg, append_to_file);

    //
    // tell the user we are logging
    //      (without putting it into the logfile)
    //
    if (!bg)
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	if (append_to_file)
	    verbose_intl(scp, i18n("appending log to $filename"));
	else
	    verbose_intl(scp, i18n("logging to $filename"));
	sub_context_delete(scp);
    }

    //
    //  make stderr go to the same place as stdout
    //      [will this work if stdout is already closed?]
    //
    close(2);
    switch (dup(1))
    {
    case 0:
	// oops, stdin is was closed
	if (dup(1) != 2)
	    fatal_raw("dup was wrong");
	close(0);
	break;

    case 2:
	break;

    default:
	nfatal("dup");
    }
    trace(("}\n"));
}


void
log_close(void)
{
    if (pid > 0)
    {
	int             status;
	int             old_pid;

	old_pid = pid;
	pid = 0;
	//
	// The tee has been created to ignore the common interrupts.
	// The only reason it will dies is if its stdin goes away.
	// Closing stdout and stderr does this.
	//
	// If we get a ^C during the waitpid (and we could, if
	// some process's child is still talking to the tee and the
	// user gets impatient) the quitter will skip this close
	// (and hence this waitpid) in the subsequent cleanup.
	//
	fclose(stdout);
	fclose(stderr);
	os_waitpid(old_pid, &status);
    }
}
