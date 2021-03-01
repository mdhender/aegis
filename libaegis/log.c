/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to open and close log files
 */

#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h> /* for umask prototype! */
#include <ac/unistd.h>
#include <ac/fcntl.h>

#include <error.h>
#include <lock.h> /* for lock_release_child */
#include <log.h>
#include <os.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static	int	pid;


static void tee_stdout _((user_ty *, char *, int, int));

static void
tee_stdout(up, filename, also_to_tty, append_to_file)
	user_ty		*up;
	char		*filename;
	int		also_to_tty;
	int		append_to_file;
{
	int		fd[2];
	int		uid;
	int		gid;
	int		um;
	char		*argv[4];
	int		argc;

	/*
	 * list both to a file and to the terminal
	 */
	trace(("tee_stdout(up = %08lX, filename = \"%s\", also_to_tty = %d, \
append_to_file = %d)\n{\n"/*}*/, (long)up, filename, also_to_tty,
append_to_file));
	uid = user_id(up);
	gid = user_gid(up);
	um = user_umask(up);
	if (pipe(fd))
		nfatal("pipe()");
	switch (pid = fork())
	{
	case 0:
		/*
		 * child
		 */
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
			/*
			 * Some systems can't write to a user's file
			 * when euid=0 over NFS.  The permissions are
			 * supposed to only apply when the file is
			 * opened, and subsequent writes are not
			 * affected.  Sigh.  Ever seen "Permission
			 * denied" from a write() call?  Eek!
			 *
			 * For systems with no functioning seteuid
			 * call, this is essential, even if NFS writes
			 * behave as they should.
			 *
			 * So: we always open a pipe, and simply run it
			 * through "tee" with the output redirected to
			 * the bit bucket.
			 */
			close(1);
			if (1 != open("/dev/null", O_WRONLY, 0666))
				nfatal("open /dev/null");
		}

		/*
		 * build the tee command
		 */
		argc = 0;
		argv[argc++] = "tee";
		if (append_to_file)
			argv[argc++] = "-a";
		argv[argc++] = filename;
		argv[argc] = 0;

		/*
		 * try to exec it
		 */
		execvp(argv[0], argv);
		nfatal("exec \"%s\"", argv[0]);

	case -1:
		nfatal("fork()");

	default:
		/*
		 * parent:
		 * redirect stdout to go through the pipe
		 */
		close(fd[0]);
		close(1);
		if (dup(fd[1]) != 1)
			fatal_raw("dup was wrong");
		close(fd[1]);
		break;
	}
	trace((/*{*/"}\n"));
}


static log_style_ty pref_to_style _((uconf_log_file_preference_ty));

static log_style_ty
pref_to_style(dflt)
	uconf_log_file_preference_ty dflt;
{
	switch (user_log_file_preference(user_executing(0), dflt))
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


/*
 *  NAME
 *	log_open - start logging
 *
 *  SYNOPSIS
 *	void log_open(string_ty *logfile, user_ty *up);
 *
 *  DESCRIPTION
 *	The log_open function is used to start sending stdout
 *	and stderr to a longfile.  If necessary it creates the log
 *	file before returning.
 */

void
log_open(log, up, style)
	string_ty	*log;
	user_ty		*up;
	log_style_ty	style;
{
	static int	already_done;
	int		bg;
	int		append_to_file;
	int		exists;

	if (already_done)
		return;
	assert(log);
	trace(("log_open(s = \"%s\")\n{\n"/*}*/, log->str_text));
	already_done = 1;

	/*
	 * check the user defaults
	 */
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
		trace((/*{*/"}\n"));
		return;
	}

	/*
	 * if the logfile exists, unlink it first
	 * (so that baseline linked to int dir works correctly)
	 */
	append_to_file = (style == log_style_append);
	user_become(up);
	exists = os_exists(log);
	if (style == log_style_snuggle && exists)
	{
		time_t	now;
		time_t	log_old, log_new;

		time(&now);
		os_mtime_range(log, &log_old, &log_new);
		if (now - log_new < 30)
			append_to_file = 1;
	}
	if (!append_to_file && exists)
		os_unlink(log);
	if (append_to_file && !exists)
		append_to_file = 0;
	user_become_undo();

	/*
	 * If we are in the background,
	 * don't send the output to the terminal.
	 */
	bg = os_background();
	tee_stdout(up, log->str_text, !bg, append_to_file);

	/*
	 * tell the user we are logging
	 *	(without putting it into the logfile)
	 */
	if (!bg)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", log);
		if (append_to_file)
			verbose_intl(scp, i18n("appending log to $filename"));
		else
			verbose_intl(scp, i18n("logging to $filename"));
		sub_context_delete(scp);
	}

	/*
	 *  make stderr go to the same place as stdout
         *      [will this work if stdout is already closed?]
	 */
        close(2);
        switch (dup(1))
        {
        case 0:
		/* oops, stdin is was closed */
		if (dup(1) != 2)
			fatal_raw("dup was wrong");
		close(0);
		break;
    
        case 2:
		break;
    
        default:
		nfatal("dup");
        }
	trace((/*{*/"}\n"));
}


void
log_close()
{
	if (pid > 0)
	{
		int	status;
		int	old_pid;

		old_pid = pid;
		pid = 0;
		/*
		 * The tee has been created to ignore the common interrupts.
		 * The only reason it will dies is if its stdin goes away.
		 * Closing stdout and stderr does this.
		 *
		 * If we get a ^C during the waitpid (and we could, if
		 * some process's child is still talking to the tee and the
		 * user gets impatient) the quitter will skip this close
		 * (and hence this waitpid) in the subsequent cleanup.
		 */
		fclose(stdout);
		fclose(stderr);
		os_waitpid(old_pid, &status);
	}
}


void
log_quitter(n)
	int	n;
{
	/*
	 * Skip the close (and the wait) if we were interrupted.
	 */
	if (!os_interrupt_has_occurred())
		log_close();
}
