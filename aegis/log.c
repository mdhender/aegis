/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to open and close log files
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <error.h>
#include <log.h>
#include <os.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static	int	pid;


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
log_open(log, up)
	string_ty	*log;
	user_ty		*up;
{
	static int	already_done;
	int		fd[2];
	char		*cmd[3];
	int		bg;

	if (already_done)
		return;
	assert(log);
	trace(("log_open(s = \"%s\")\n{\n"/*}*/, log->str_text));
	already_done = 1;

	/*
	 * if the logfile exists, unlink it first
	 * (so that baseline linked to int dir works correctly)
	 */
	user_become(up);
	if (os_exists(log))
		os_unlink(log);
	user_become_undo();

	/*
	 * If we are in the background,
	 * don't send the output to the terminal.
	 */
	bg = os_background();
	if (bg)
	{
		user_become(up);
		if (!freopen(log->str_text, "w", stdout))
			nfatal("%s", log->str_text);
		user_become_undo();
	}
	else
	{
		int	uid;
		int	gid;
		int	um;

		/*
		 * list both to a file and to the terminal
		 */
		uid = user_id(up);
		gid = user_gid(up);
		um = user_umask(up);
		if (pipe(fd))
			nfatal("pipe()");
		switch (pid = fork())
		{
		case 0:
			undo_cancel();
			while (os_become_active())
				os_become_undo();
			cmd[0] = "tee";
			cmd[1] = log->str_text;
			cmd[2] = 0;
			close(fd[1]);
			close(0);
			if (dup(fd[0]) != 0)
				fatal("dup was wrong");
			close(fd[0]);
			signal(SIGINT, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			signal(SIGTERM, SIG_IGN);
			os_setgid(gid);
			os_setuid(uid);
			umask(um);
			execvp(cmd[0], cmd);
			nfatal("%s", cmd[0]);
	
		case -1:
			nfatal("fork()");
	
		default:
			close(fd[0]);
			close(1);
			if (dup(fd[1]) != 1)
				fatal("dup was wrong");
			close(fd[1]);
			break;
		}
	}

	/*
	 * tell the user we are logging
	 *	(without putting it into the logfile)
	 */
	if (!bg)
		verbose("logging to \"%s\"", log->str_text);

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
			fatal("dup was wrong");
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

		fclose(stdout);
		fclose(stderr);
		os_waitpid(pid, &status);
		pid = 0;
	}
}


void
log_quitter(n)
	int	n;
{
	log_close();
}
