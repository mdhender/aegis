/*
 *	aegis - project change supervisor
 *	Copyright (C) 1992, 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to pipe output through paginator
 */

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>

#include <env.h>
#include <error.h>
#include <option.h>
#include <os.h>
#include <pager.h>
#include <trace.h>
#include <undo.h>


static	FILE	*out;
static	char	*pager;
static	int	pid;


static void env_set_page _((void));

static void
env_set_page()
{
	char		buffer[20];

	sprintf(buffer, "%d", option_page_length_get());
	env_set("LINES", buffer);
	sprintf(buffer, "%d", option_page_width_get());
	env_set("COLS", buffer);
}


static FILE *pipe_open _((char *));

static FILE *
pipe_open(prog)
	char		*prog;
{
	int	uid;
	int	gid;
	int	um;
	FILE	*fp;
	int	fd[2];
	char	*cmd[4];

	fp = 0;
	os_become_orig_query(&uid, &gid, &um);
	if (pipe(fd))
		nfatal("pipe()");
	switch (pid = fork())
	{
	case 0:
		env_set_page();
		undo_cancel();
		while (os_become_active())
			os_become_undo();
		cmd[0] = "sh";
		cmd[1] = "-c";
		cmd[2] = prog;
		cmd[3] = 0;
		close(fd[1]);
		close(0);
		if (dup(fd[0]) != 0)
			fatal("dup was wrong");
		close(fd[0]);
		os_setgid(gid);
		os_setuid(uid);
		umask(um);
		execvp(cmd[0], cmd);
		nfatal("%s", prog);

	case -1:
		nerror("fork()");
		fp = 0;
		break;

	default:
		close(fd[0]);
		fp = fdopen(fd[1], "w");
		if (!fp)
			nfatal("fdopen");
		break;
	}
	return fp;
}


static void pipe_close _((FILE *));

static void
pipe_close(fp)
	FILE		*fp;
{
	int		status;

	fclose(fp);
	os_waitpid(pid, &status);
	pid = 0;
}


static void cleanup _((int));

static void
cleanup(n)
	int		n;
{
	trace(("cleanup(n = %d)\n{\n"/*}*/, n));
	if (!out)
		goto done;

	/*
	 * write the last of the output
	 */
	fflush(out);

	/*
	 * close the paginator
	 */
	if (pager)
	{
		pipe_close(out);
		pager = 0;
	}
	out = 0;

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


FILE *
pager_open()
{
	trace(("pager_open()\n{\n"/*}*/));
	assert(!out);

	/*
	 * if talking to a terminal,
	 * send the output through a paginator
	 */
	if
	(
		!option_unformatted_get()
	&&
		!os_background()
	&&
		isatty(0)
	&&
		isatty(1)
	)
	{
		pager = getenv("PAGER");
		if (!pager || !*pager)
			pager = "more";
	}
	else
		pager = 0;

	/*
	 * register the cleanup function in case of fatal errors
	 */
	quit_register(cleanup);

	/*
	 * open the paginator
	 */
	if (pager)
	{
		os_become_orig();
		out = pipe_open(pager);
		os_become_undo();
		if (!out)
		{
			pager = 0;
			out = stdout;
		}
	}
	else
		out = stdout;
	trace((/*{*/"}\n"));
	return out;
}


void
pager_close(fp)
	FILE		*fp;
{
	trace(("pager_close(fp = %08lX)\n{\n"/*}*/, fp));
	assert(out);
	assert(fp == out);

	/*
	 * write the last of the output
	 */
	fflush(out);
	if (ferror(out))
		pager_error(out);

	/*
	 * close the paginator
	 */
	if (pager)
	{
		pipe_close(out);
		pager = 0;
	}
	out = 0;
	trace((/*{*/"}\n"));
}


void
pager_error(fp)
	FILE		*fp;
{
	assert(out);
	assert(fp == out);

	if (pager)
		nfatal("writing pipe to \"%s\"", pager);
	else
		nfatal("standard output");
}
