/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
 * MANIFEST: wrappers around operating system functions
 */

#include <ac/stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <ac/string.h>
#include <ac/limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <ac/unistd.h>
#include <utime.h>
#include <pwd.h>
#include <grp.h>

#include <dir.h>
#include <error.h>
#include <file.h>
#include <glue.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <trace.h>
#include <undo.h>
#include <word.h>

#define MAX_CMD_RPT 36


static	int	interrupted;
static	int	become_orig_uid;
static	int	become_orig_gid;
static	int	become_orig_umask;
static	int	become_inited;
static	int	become_testing;
static	int	become_active;
static	int	become_active_uid;
static	int	become_active_gid;
static	int	become_active_umask;


static RETSIGTYPE interrupt _((int));

static RETSIGTYPE
interrupt(n)
	int	n;
{
	interrupted++;
	signal(n, interrupt);
}


int
os_waitpid_status(child, cmd)
	int		child;
	char		*cmd;
{
	int		result;
	RETSIGTYPE	(*hold)_((int));
	int		a, b, c;
	int		status;

	trace(("os_waitpid_status(child = %d, cmd = \"%s\")\n{\n"/*}*/, child, cmd));
	interrupted = 0;
	hold = signal(SIGINT, SIG_IGN);
	if (hold != SIG_IGN)
		signal(SIGINT, interrupt);
	result = 0;
	if (os_waitpid(child, &status))
		nfatal("wait");
	a = (status >> 8) & 0xFF;
	b = (status >> 7) & 1;
	c = status & 0x7F;
	switch (c)
	{
	case 0x7F:
		/*
		 * process was stopped,
		 * since we didn't do it, treat it as an error
		 */
		fatal("command \"%s\" stopped", cmd);

	case 0:
		/* normal termination */
		result = a;
		break;

	default:
		/*
		 * process dies from unhandled condition
		 */
		fatal
		(
			"command \"%s\" terminated by %s%s",
			cmd,
			signal_name(c),
			(b ? " (core dumped)" : "")
		);
	}
	if (hold != SIG_IGN)
		signal(SIGINT, hold);
	if (interrupted)
		fatal("interrupted");
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


int
os_exists(path)
	string_ty	*path;
{
	struct stat	st;
	int		oret;

	os_become_must_be_active();
#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
	{
		if (errno != ENOENT)
			nfatal("stat(\"%s\")", path->str_text);
		return 0;
	}
	return 1;
}


void
os_mkdir(path, mode)
	string_ty	*path;
	int		mode;
{
	int		uid;
	int		gid;
	int		um;

	trace(("os_mkdir(path = \"%s\", mode = 0%o)\n{\n"/*}*/,
		path->str_text, mode));
	os_become_must_be_active();
	if (glue_mkdir(path->str_text, mode))
		nfatal("mkdir(\"%s\", 0%o)", path->str_text, mode);

	/*
	 * There could be Berkeley semantics about the group
	 * of the newly created directory, so make sure it is
	 * the one intended (egid).
	 */
	os_become_query(&uid, &gid, &um);
	if (glue_chown(path->str_text, uid, gid))
		nfatal("chgrp(\"%s\", %d)", path->str_text, gid);

	/*
	 * The set-group-id bit is ignored by a Berkeley semantics mkdir
	 * (and it would be nuked the the chgrp, anyway)
	 * so set it explicitly.
	 */
	mode &= ~um;
	if (glue_chmod(path->str_text, mode))
		nfatal("chmod(\"%s\", 0%0)", path->str_text, mode);
	trace((/*{*/"}\n"));
}


void
os_rmdir(path)
	string_ty	*path;
{
	trace(("os_rmdir(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_rmdir(path->str_text))
		nfatal("rmdir(\"%s\")", path->str_text);
	trace((/*{*/"}\n"));
}


void
os_rmdir_errok(path)
	string_ty	*path;
{
	trace(("os_rmdir_errok(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_rmdir(path->str_text))
		nerror("warning: rmdir(\"%s\")", path->str_text);
	trace((/*{*/"}\n"));
}


void
os_mkdir_between(top, extn, mode)
	string_ty	*top;
	string_ty	*extn;
	int		mode;
{
	char		*cp;
	string_ty	*s1;
	string_ty	*s2;

	trace(("os_mkdir_between(top = %08lX, extn = %08lX, \
mode = 0%o)\n{\n"/*}*/, top, extn, mode));
	trace_string(top->str_text);
	trace_string(extn->str_text);
	assert(top->str_text[0] == '/');
	assert(extn->str_text[0] != '/');
	assert(extn->str_text[extn->str_length - 1] != '/');
	assert(extn->str_length);
	for (cp = extn->str_text; *cp; ++cp)
	{
		if (*cp != '/')
			continue;
		s1 = str_n_from_c(extn->str_text, cp - extn->str_text);
		s2 = str_format("%S/%S", top, s1);
		if (!os_exists(s2))
		{
			os_mkdir(s2, mode);
			undo_rmdir_errok(s2);
		}
		str_free(s1);
		str_free(s2);
	}
	trace((/*{*/"}\n"));
}


void
os_rename(a, b)
	string_ty	*a;
	string_ty	*b;
{
	trace(("os_rename(a = %08lX, b = %08lX)\n{\n"/*}*/, a, b));
	os_become_must_be_active();
	trace_string(a->str_text);
	trace_string(b->str_text);
	if (glue_rename(a->str_text, b->str_text))
		nfatal("rename(\"%s\", \"%s\")", a->str_text, b->str_text);
	trace((/*{*/"}\n"));
}

void
os_unlink(path)
	string_ty	*path;
{
	struct stat	st;
	int		oret;

	trace(("os_unlink(path = %08lX)\n{\n"/*}*/, path));
	os_become_must_be_active();
	trace_string(path->str_text);

	/*
	 * We must check that we are not unlinking a directory,
	 * because we are set-uid-root, and root can unlink directories!
	 */
#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
		nfatal("stat(\"%s\")", path->str_text);
	if ((st.st_mode & S_IFMT) == S_IFDIR)
		fatal("unlink(\"%s\"): is a directory", path->str_text);
	if (glue_unlink(path->str_text))
		nfatal("unlink(\"%s\")", path->str_text);
	trace((/*{*/"}\n"));
}

void
os_unlink_errok(path)
	string_ty	*path;
{
	struct stat	st;
	int		oret;

	trace(("os_unlink_errok(path = %08lX)\n{\n"/*}*/, path));
	os_become_must_be_active();
	trace_string(path->str_text);

	/*
	 * We must check that we are not unlinking a directory,
	 * because we are set-uid-root, and root can unlink directories!
	 */
#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
	{
		if (errno != ENOENT)
			nfatal("stat(\"%s\")", path->str_text);
		nerror("warning: stat(\"%s\")", path->str_text);
		goto done;
	}
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		error
		(
		 "warning: unlink(\"%s\"): is a directory, no action performed",
			path->str_text
		);
		goto done;
	}
	if (glue_unlink(path->str_text))
		nerror("warning: unlink(\"%s\")", path->str_text);

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	os_curdir_sub - get current directory
 *
 * SYNOPSIS
 *	string_ty *os_curdir_sub(void);
 *
 * DESCRIPTION
 *	The os_curdir_sub function is used to determine the system's idea
 *	of the current directory.
 *
 * RETURNS
 *	A pointer to a string in dynamic memory is returned.
 *	A null pointer is returned on error.
 *
 * CAVEAT
 *	DO NOT use str_free() on the value returned.
 */

static string_ty *os_curdir_sub _((void));

static string_ty *
os_curdir_sub()
{
	static string_ty	*s;

	os_become_must_be_active();
	if (!s)
	{
		char	buffer[2000];

		if (!glue_getcwd(buffer, sizeof(buffer)))
			nfatal("getcwd");
		assert(buffer[0] == '/');
		s = str_from_c(buffer);
	}
	return s;
}


/*
 * NAME
 *	os_curdir - full current directory path
 *
 * SYNOPSIS
 *	string_ty *os_curdir(void);
 *
 * DESCRIPTION
 *	Os_curdir is used to determine the pathname
 *	of the current directory.  Automounter vaguaries will be elided.
 *
 * RETURNS
 *	A pointer to a string in dynamic memory is returned.
 *	A null pointer is returned on error.
 *
 * CAVEAT
 *	Use str_free() when you are done with the value returned.
 */

string_ty *
os_curdir()
{
	static string_ty	*result;

	os_become_must_be_active();
	if (!result)
	{
		string_ty	*dot;
		
		dot = str_from_c(".");
		result = os_pathname(dot, 1);
		str_free(dot);
	}
	return str_copy(result);
}


/*
 * NAME
 *	os_pathname - determine full file name
 *
 * SYNOPSIS
 *	string_ty *os_pathname(string_ty *path, int resolve);
 *
 * DESCRIPTION
 *	Os_pathname is used to determine the full path name
 *	of a partial path given.
 *
 * ARGUMENTS
 *	path	- path to canonicalize
 *	resolve	- non-zero if should resolve symlinks, 0 if not
 *
 * RETURNS
 *	pointer to dynamically allocated string.
 *
 * CAVEAT
 *	Use str_free() when you are done with the value returned.
 */

string_ty *
os_pathname(path, resolve)
	string_ty	*path;
	int		resolve;
{
	static char	*tmp;
	static size_t	tmplen;
	static size_t	ipos;
	static size_t	opos;
	int		c;
	int		found;
#ifdef S_IFLNK
	int		loop;
#endif

	/*
	 * Change relative pathnames to absolute
	 */
	trace(("os_pathname(path = %08lX)\n{\n"/*}*/, path));
	if (resolve)
		os_become_must_be_active();
	trace_string(path->str_text);
	if (path->str_text[0] != '/')
		path = str_format("%S/%S", os_curdir_sub(), path);
	else
		path = str_copy(path);
	if (!tmp)
	{
		tmplen = 200;
		tmp = mem_alloc(tmplen);
	}

	/*
	 * Take kinks out of the pathname
	 */
	ipos = 0;
	opos = 0;
	found = 0;
#ifdef S_IFLNK
	loop = 0;
#endif
	while (!found)
	{
		/*
		 * get the next character
		 */
		c = path->str_text[ipos];
		if (c)
			ipos++;
		else
		{
			found = 1;
			c = '/';
		}

		/*
		 * remember the normal characters
		 * until get to slash
		 */
		if (c != '/')
			goto remember;

		/*
		 * leave root alone
		 */
		if (!opos)
			goto remember;

		/*
		 * "/.." -> "/"
		 */
		if (opos == 3 && tmp[1] == '.' && tmp[2] == '.')
		{
			opos = 1;
			continue;
		}

		/*
		 * "a//" -> "a/"
		 */
		if (tmp[opos - 1] == '/')
			continue;

		/*
		 * "a/./" -> "a/"
		 */
		if (opos >= 2 && tmp[opos - 1] == '.' && tmp[opos - 2] == '/')
		{
			opos--;
			continue;
		}

		/*
		 * "a/b/../" -> "a/"
		 */
		if
		(
			opos > 3
		&&
			tmp[opos - 1] == '.'
		&&
			tmp[opos - 2] == '.'
		&&
			tmp[opos - 3] == '/'
		)
		{
			opos -= 4;
			assert(opos > 0);
			while (tmp[opos - 1] != '/')
				opos--;
			continue;
		}

		/*
		 * see if the path so far is a symbolic link
		 */
#ifdef S_IFLNK
		if (resolve)
		{
			char		pointer[2000];
			int		nbytes;
			string_ty	*s;

			s = str_n_from_c(tmp, opos);
			nbytes = glue_readlink(s->str_text, pointer, sizeof(pointer) - 1);
			if (nbytes < 0)
			{
				/*
				 * probably not a symbolic link
				 */
				if
				(
					errno != ENXIO
				&&
					errno != EINVAL
				&& 
					errno != ENOENT
				)
					nfatal("readlink(\"%s\")", s->str_text);
				str_free(s);
			}
			else
			{
				string_ty	*newpath;
	
				if (nbytes == 0)
				{
					fatal
					(
					       "readlink(\"%s\") returned \"\"",
						s->str_text
					);
				}
				if (++loop > 1000)
				{
					fatal
					(
					   "symbolic link loop \"%s\" detected",
						s->str_text
					);
				}
				pointer[nbytes] = 0;
	
				str_free(s);
				if (pointer[0] == '/')
					tmp[1] = 0;
				else
				{
					while (tmp[opos - 1] != '/')
						opos--;
					tmp[opos] = 0;
				}
				newpath =
					str_format
					(
						"%s/%s/%s",
						tmp,
						pointer,
						path->str_text + ipos
					);
				str_free(path);
				path = newpath;
				ipos = 0;
				opos = 0;
				found = 0;
				continue;
			}
		}
#endif
	
		/*
		 * keep the slash
		 */
		remember:
		if (opos >= tmplen)
		{
			tmplen += 100;
			tmp = mem_change_size(tmp, tmplen);
		}
		tmp[opos++] = c;
	}
	str_free(path);
	assert(opos >= 1);
	assert(tmp[0] == '/');
	assert(tmp[opos - 1] == '/');
	if (opos >= 2)
		opos--;
	path = str_n_from_c(tmp, opos);
	trace_string(path->str_text);
	trace((/*{*/"}\n"));
	return path;
}


/*
 * NAME
 *	os_entryname - take path apart
 *
 * SYNOPSIS
 *	string_ty *os_entryname(string_ty *path);
 *
 * DESCRIPTION
 *	Os_entryname is used to extract the entry part
 *	from a pathname.
 *
 * RETURNS
 *	pointer to dynamically allocated string.
 *
 * CAVEAT
 *	Use str_free() when you are done with the return value.
 */

string_ty *
os_entryname(path)
	string_ty	*path;
{
	string_ty	*s;
	char		*cp;

	trace(("os_entryname(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	s = os_pathname(path, 1);
	cp = strrchr(s->str_text, '/');
	if (cp && s->str_length > 1)
		path = str_from_c(cp + 1);
	else
		path = str_copy(s);
	str_free(s);
	trace_string(path->str_text);
	trace((/*{*/"}\n"));
	return path;
}


/*
 * NAME
 *	os_dirname - take path apart
 *
 * SYNOPSIS
 *	string_ty *os_dirname(string_ty *path);
 *
 * DESCRIPTION
 *	Os_dirname is used to extract the directory part
 *	of a pathname.
 *
 * RETURNS
 *	pointer to dynamically allocated string.
 *
 * CAVEAT
 *	Use str_free() when you are done with the value returned.
 */

string_ty *
os_dirname(path)
	string_ty	*path;
{
	string_ty	*s;
	char		*cp;

	trace(("os_dirname(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	s = os_pathname(path, 1);
	cp = strrchr(s->str_text, '/');
	assert(cp);
	if (cp > s->str_text)
	{
		path = str_n_from_c(s->str_text, cp - s->str_text);
		str_free(s);
	}
	else
		path = s;
	trace_string(path->str_text);
	trace((/*{*/"}\n"));
	return path;
}


string_ty *
os_below_dir(higher, lower)
	string_ty	*higher;
	string_ty	*lower;
{
	char		*s1;
	char		*s2;

	s1 = higher->str_text;
	s2 = lower->str_text;
	while (*s1 == *s2 && *s1)
		s1++, s2++;
	if (*s1)
		return 0;
	if (!*s2)
		return str_from_c("");
	if (*s2 != '/')
		return 0;
	return str_from_c(s2 + 1);
}


void
os_chdir(path)
	string_ty	*path;
{
	os_become_must_not_be_active();
	if (chdir(path->str_text))
		nfatal("chdir(\"%s\")", path->str_text);
}


void
os_setuid(uid)
	int		uid;
{
	os_become_must_not_be_active();
	if (setuid(uid))
		nfatal("setuid(%d)", uid);
}


void
os_setgid(gid)
	int		gid;
{
	os_become_must_not_be_active();
	if (setgid(gid))
	{
		if (become_testing)
		{
			/*
			 * don't use os_testing_mode(),
			 * it could mess with errno
			 */
			nerror("warning: setgid(%d)", gid);
		}
		else
			nfatal("setgid(%d)", gid);
	}
}


void
os_execute(cmd, flags, dir)
	string_ty	*cmd;
	int		flags;
	string_ty	*dir;
{
	int		result;

	trace(("os_execute()\n{\n"/*}*/));
	result = os_execute_retcode(cmd, flags, dir);
	if (result)
	{
		string_ty	*s;

		if (cmd->str_length > MAX_CMD_RPT)
			s = str_format("%.*S...", MAX_CMD_RPT - 3, cmd);
		else
			s = str_copy(cmd);
		fatal("command \"%s\" exit status: %d", s->str_text, result);
		str_free(s);
	}
	trace((/*{*/"}\n"));
}


static void who_and_where _((int, int, string_ty *));

static void
who_and_where(uid, gid, dir)
	int		uid;
	int		gid;
	string_ty	*dir;
{
	static int	last_uid;
	static int	last_gid;
	static string_ty *last_dir;
	string_ty	*rdir;

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
		str_free(last_dir);
		last_dir = rdir;
		error("cd %s", dir->str_text);
	}
	else
		str_free(rdir);

	if (last_uid != uid || last_gid != gid)
	{
		struct passwd	*pw;
		char		user[20];
		struct group	*gr;
		char		group[20];

		last_uid = uid;
		pw = getpwuid(uid);
		if (pw)
		{
			sprintf
			(
				user,
				"\"%.*s\"",
				(int)(sizeof(user) - 3),
				pw->pw_name
			);
		}
		else
			sprintf(user, "%d", uid);

		last_gid = gid;
		gr = getgrgid(gid);
		if (gr)
		{
			sprintf
			(
				group,
				"\"%.*s\"",
				(int)(sizeof(group) - 3),
				gr->gr_name
			);
		}
		else
			sprintf(group, "%d", gid);
		error("user %s, group %s", user, group);
	}
}


int
os_execute_retcode(cmd, flags, dir)
	string_ty	*cmd;
	int		flags;
	string_ty	*dir;
{
	int		uid;
	int		gid;
	int		um;
	int		child;
	int		result = 0;
	RETSIGTYPE	(*hold)_((int));
	string_ty	*cmd2;
	char		*shell;

	trace(("os_execute_retcode()\n{\n"/*}*/));
	os_become_must_be_active();
	os_become_query(&uid, &gid, &um);
	who_and_where(uid, gid, dir);
	if (cmd->str_length > MAX_CMD_RPT)
		cmd2 = str_format("%.*S...", MAX_CMD_RPT - 3, cmd);
	else
		cmd2 = str_copy(cmd);

	/*
	 * fork to get a process to do the command
	 */
	switch (child = fork())
	{
	case -1:
		nfatal("fork");

	default:
		/*
		 * The parent process waits
		 */
		interrupted = 0;
		hold = signal(SIGINT, SIG_IGN);
		if (hold != SIG_IGN)
			signal(SIGINT, interrupt);
		result = os_waitpid_status(child, cmd2->str_text);
		break;

	case 0:
		/*
		 * become the user for real
		 */
		while (os_become_active())
			os_become_undo();
		undo_cancel();
		os_setgid(gid);
		os_setuid(uid);
		umask(um);

		/*
		 * change directory to the appropriate directory.
		 */
		os_chdir(dir);

		/*
		 * Redirect stdin from a broken pipe.
		 * (Don't redirect stdin if not logging, for manual tests.)
		 */
		if (!(flags & OS_EXEC_FLAG_INPUT))
		{
			int	pfd[2];
			int	n;

			if (pipe(pfd))
				nfatal("pipe");
			if (close(0))
				nfatal("close stdin");
			n = dup(pfd[0]);
			if (n < 0)
				nfatal("dup");
			if (n != 0)
				fatal("dup gave %d, not 0 (bug)", n);
			if (close(pfd[0]) || close(pfd[1]))
				nfatal("close pipe ends");
		}

		/*
		 * let the log file (user) know what we did
		 */
		error("%s", cmd->str_text);

		/*
		 * invoke the command through sh(1)
		 */
		shell = os_shell();
		execl(shell, shell, "-ec", cmd->str_text, (char *)0);
		nfatal("exec(\"%s\")", shell);
	}
	if (result && (flags & OS_EXEC_FLAG_ERROK))
	{
		error
		(
			"warning: command \"%s\" exit status %d",
			cmd2->str_text,
			result
		);
		result = 0;
	}
	str_free(cmd2);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


time_t
os_mtime(path)
	string_ty	*path;
{
	struct stat	st;
	time_t		result;
	int		oret;

	trace(("os_mtime(path = %08lX)\n{\n"/*}*/, path));
	os_become_must_be_active();
	trace_string(path->str_text);

#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
		nfatal("stat(\"%s\")", path->str_text);

	/*
	 * Return the last modified time.
	 * They may try to fake us out by using utime,
	 * so check the inode change time, too.
	 */
	if (st.st_ctime > st.st_mtime)
		result = st.st_ctime;
	else
		result = st.st_mtime;
	trace(("return %s", ctime(&result)));
	trace((/*{*/"}\n"));
	return result;
}


void
os_mtime_set(path, when)
	string_ty	*path;
	time_t		when;
{
	struct utimbuf	utb;

	trace(("os_mtime_set(path = %08lX, when = %ld)\n{\n"/*}*/, path, when));
	os_become_must_be_active();
	trace_string(path->str_text);
	trace(("when = %s", ctime(&when)));
	utb.actime = when;
	utb.modtime = when;
	if (glue_utime(path->str_text, &utb))
		nfatal("utime(\"%s\")", path->str_text);
	trace((/*{*/"}\n"));
}


void
os_chown_check(path, mode, uid, gid)
	string_ty	*path;
	int		mode;
	int		uid;
	int		gid;
{
	struct stat	st;
	int		nerrs;
	int		oret;

	trace(("os_chown_check(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	nerrs = 0;
#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
		nfatal("stat(\"%s\")", path->str_text);
	if (os_testing_mode())
	{
		if ((st.st_mode & 07777) != mode)
		{
			error
			(
				"%s: mode is %05o, should be %05o",
				path->str_text,
				st.st_mode & 07777,
				mode
			);
			++nerrs;
		}
		if (st.st_uid != geteuid())
		{
			error
			(
				"%s: owner is %d, should be %d",
				path->str_text,
				st.st_uid,
				geteuid()
			);
			++nerrs;
		}
	}
	else
	{
		if ((st.st_mode & 07777) != mode)
		{
			error
			(
				"%s: mode is %05o, should be %05o",
				path->str_text,
				st.st_mode & 07777,
				mode
			);
			++nerrs;
		}
		if (st.st_uid != uid)
		{
			error
			(
				"%s: owner is %d, should be %d",
				path->str_text,
				st.st_uid,
				uid
			);
			++nerrs;
		}
		if (gid >= 0 && st.st_gid != gid)
		{
			error
			(
				"%s: group is %d, should be %d",
				path->str_text,
				st.st_gid,
				gid
			);
			++nerrs;
		}
	}
	if (nerrs)
		fatal("%s: has been tampered with (fatal)", path->str_text);
	trace((/*{*/"}\n"));
}


void
os_chmod(path, mode)
	string_ty	*path;
	int		mode;
{
	trace(("os_chmod(path = \"%s\", mode = 0%o)\n{\n"/*}*/,
		path->str_text, mode));
	mode &= 07777;
	os_become_must_be_active();
	if (glue_chmod(path->str_text, mode))
		nfatal("chmod(\"%s\", 0%o)", path->str_text, mode);
	trace((/*{*/"}\n"));
}


int
os_chmod_query(path)
	string_ty	*path;
{
	int		mode;
	struct stat	st;
	int		oret;

	trace(("os_chmod_query(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
		nfatal("stat(\"%s\")", path->str_text);
	mode = st.st_mode & 07777;
	trace(("return %05o;\n", mode));
	trace((/*{*/"}\n"));
	return mode;
}


void
os_chmod_errok(path, mode)
	string_ty	*path;
	int		mode;
{
	trace(("os_chmod_errok(path = \"%s\", mode = 0%o)\n{\n"/*}*/,
		path->str_text, mode));
	os_become_must_be_active();
	mode &= 07777;
	if (glue_chmod(path->str_text, mode))
		nerror("warning: chmod(\"%s\", 0%o)", path->str_text, mode);
	trace((/*{*/"}\n"));
}


void
os_link(from, to)
	string_ty	*from;
	string_ty	*to;
{
	trace(("os_link(from = %08lX, to = %08lX)\n{\n"/*}*/, from, to));
	os_become_must_be_active();
	trace_string(from->str_text);
	trace_string(to->str_text);
	if (glue_link(from->str_text, to->str_text))
		nfatal("link(\"%s\", \"%s\")", from->str_text, to->str_text);
	trace((/*{*/"}\n"));
}


string_ty *
os_execute_slurp(cmd, flags, dir)
	string_ty	*cmd;
	int		flags;
	string_ty	*dir;
{
	string_ty	*s1;
	string_ty	*s2;

	trace(("os_execute_slurp()\n{\n"/*}*/));
	s1 = os_edit_filename(0);
	trace_string(s1->str_text);
	s2 = str_format("( %S ) > %S", cmd, s1);
	os_execute(s2, flags, dir);
	str_free(s2);
	s2 = read_whole_file(s1->str_text);
	os_unlink(s1);
	str_free(s1);
	trace_string(s2->str_text);
	trace((/*{*/"}\n"));
	return s2;
}


static void whoami _((int *, int *));

static void
whoami(uid, gid)
	int	*uid;
	int	*gid;
{
	*uid = become_active_uid;
	*gid = become_active_gid;
}


void
os_become_init()
{
	assert(!become_inited);
	error_set_id_func(whoami);
	become_active_umask = DEFAULT_UMASK;
	become_orig_umask = umask(DEFAULT_UMASK);
	become_orig_uid = getuid();
	become_orig_gid = getgid();
	if (setuid(0))
	{
		become_testing = 1;
		become_active_uid = become_orig_uid;
		become_active_gid = become_orig_gid;
	}
	else
		setgid(0);
	become_inited = 1;
}


int
os_testing_mode()
{
	static int warned;

	assert(become_inited);
	if (!become_testing)
		return 0;
	if (!warned)
	{
		warned = 1;
		verbose
		(
"warning: \
This copy is running in \"test\" mode.  \
It has reduced capabilities due to reduced privileges.  \
This mode is provided solely for testing, \
and is not a recommended operating mode."
		);
	}
	return 1;
}


void
os_become(uid, gid, um)
	int	uid;
	int	gid;
	int	um;
{
	trace(("os_become(uid = %d, gid = %d, um = %03o)\n{\n"/*}*/, uid, gid, um));
	if (become_active)
		fatal("multiple user permissions set (bug)");
	become_active = 1;
	if (os_testing_mode())
	{
		become_active_umask = um;
		umask(um);
	}
	else
	{
		become_active_uid = uid;
		become_active_gid = gid;
		become_active_umask = um;
#ifndef CONF_NO_seteuid
		if (setegid(gid))
			nfatal("setegid(%d)", gid);
		if (seteuid(uid))
			nfatal("seteuid(%d)", uid);
		umask(um);
#endif
	}
	trace((/*{*/"}\n"));
}


void
os_become_undo()
{
	trace(("os_become_undo()\n{\n"/*}*/));
	os_become_must_be_active();
	assert(become_inited);
	become_active = 0;
	if (!become_testing)
	{
#ifndef CONF_NO_seteuid
		if (seteuid(0))
			nfatal("seteuid(0)");
		if (setegid(0))
			nfatal("setegid(0)");
#endif
		become_active_uid = 0;
		become_active_gid = 0;
	}
	trace((/*{*/"}\n"));
}


void
os_become_orig()
{
	os_become(become_orig_uid, become_orig_gid, become_orig_umask);
}


void
os_become_query(uid, gid, umsk)
	int	*uid;
	int	*gid;
	int	*umsk;
{
	os_become_must_be_active();
	*uid = become_active_uid;
	if (gid)
		*gid = become_active_gid;
	if (umsk)
		*umsk = become_active_umask;
}


void
os_become_orig_query(uid, gid, umsk)
	int	*uid;
	int	*gid;
	int	*umsk;
{
	if (uid)
		*uid = become_orig_uid;
	if (gid)
		*gid = become_orig_gid;
	if (umsk)
		*umsk = become_orig_umask;
}


int
os_become_active()
{
	return become_active;
}


void
os_become_must_be_active_gizzards(file, line)
	char	*file;
	int	line;
{
	if (!os_become_active())
		fatal("%s: %d: user permissions not set (bug)", file, line);
}


void
os_become_must_not_be_active_gizzards(file, line)
	char	*file;
	int	line;
{
	if (os_become_active())
	{
		fatal
		(
			"%s: %d: user permissions set when aught not (bug)",
			file,
			line
		);
	}
}


#ifdef SIGSTOP
#ifndef HAVE_TCGETPGRP

#include <sys/termio.h>

int
tcgetpgrp(fd)
	int		fd;
{
	int		result;

#ifdef TIOCGETPGRP
	if (ioctl(fd, TIOCGETPGRP, &result))
		result = -1;
#else
#ifdef TIOCGPGRP
        if (ioctl(fd, TIOCGPGRP, &result))
		result = -1;
#else
	result = -1;
#endif
#endif
	return result;
}

#endif /* !HAVE_TCGETPGRP */
#endif /* SIGSTOP */


/*
 *  NAME
 *	  background - test for backgroundness
 *
 *  SYNOPSIS
 *	  int background(void);
 *
 *  DESCRIPTION
 *	  The background function is used to determin e if the curent process is
 *	  in the background.
 *
 *  RETURNS
 *	  int: zero if process is not in the background, nonzero if the process
 *	  is in the background.
 *
 * CAVEAT:
 *	This function has a huge chance of being wrong for your system.
 *	If you need to modify this function, please let the author know.
 */

int
os_background()
{
	RETSIGTYPE	(*x)_((int));

	/*
	 * C shell
	 *	puts its children in a different process group.
	 *	The process group the terminal in is the forground.
	 *
	 * Only available on systems with job control.
	 */
#ifdef SIGSTOP
	if (getpgrp(CONF_getpgrp_arg) != tcgetpgrp(0))
		return 1;
#endif

	/*
	 * Bourne shell
	 *	sets its children to ignore SIGINT
	 */
	x = signal(SIGINT, SIG_IGN);
	if (x == SIG_IGN)
		return 1;
	signal(SIGINT, x);

	/*
	 * probably forground
	 */
	return 0;
}


int
os_readable(path)
	string_ty	*path;
{
	int		fd;
	os_become_must_be_active();
	fd = glue_open(path->str_text, 0, 0666);
	if (fd < 0)
		return errno;
	glue_close(fd);
	return 0;
}


int
os_waitpid(child, status_p)
	int		child;
	int		*status_p;
{
	typedef struct ret_ty ret_ty;
	struct ret_ty
	{
		int	pid;
		int	status;
	};

	static long	nret;
	static long	nret_max;
	static ret_ty	*ret;
	int		pid;
	int		status;
	int		j;
	int		result;

	/*
	 * see if we already have it
	 */
	trace(("os_waitpid(child = %d)\n{\n"/*}*/, child));
	assert(child > 0);
	result = 0;
	for (j = 0; j < nret; ++j)
	{
		if (ret[j].pid != child)
			continue;
		*status_p = ret[j].status;
		ret[j] = ret[--nret];
		goto done;
	}

	/*
	 * new one, go hunting
	 */
	for (;;)
	{
		/*
		 * block until a child terminates,
		 * or there are no more children
		 */
		pid = wait(&status);
		if (pid == -1)
		{
			if (errno == EINTR)
				continue;
			result = -1;
			break;
		}

		/*
		 * stop if this is the child
		 * we are looking for
		 */
		if (pid == child)
		{
			*status_p = status;
			break;
		}

		/*
		 * remember and keep going
		 */
		if (nret >= nret_max)
		{
			long	nbytes;

			nret_max += 11;
			nbytes = nret_max * sizeof(ret_ty);
			if (!ret)
				ret = mem_alloc(nbytes);
			else
				ret = mem_change_size(ret, nbytes);
		}
		ret[nret].pid = pid;
		ret[nret].status = status;
		++nret;
	}

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


char *
os_shell()
{
	static char shell[] = CONF_SHELL;

	assert(shell[0] == '/');
	return shell;
}


void
os_edit(filename)
	string_ty	*filename;
{
	string_ty	*cmd;
	string_ty	*cwd;
	char		*editor;

	/*
	 * make sure we are in a position to edit
	 */
	if (os_background())
		fatal("may not use -Edit in the background");

	/*
	 * find the editor to use
	 */
	editor = getenv("EDITOR");
	if (!editor)
		editor = "vi";

	/*
	 * edit the file
	 */
	cmd = str_format("%s %S", editor, filename);
	cwd = os_curdir();
	assert(cwd);
	os_execute(cmd, OS_EXEC_FLAG_INPUT | OS_EXEC_FLAG_ERROK, cwd);
	str_free(cmd);
	str_free(cwd);
}


string_ty *
os_edit_new()
{
	string_ty	*filename;
	string_ty	*result;
	FILE		*fp;

	filename = os_edit_filename(0);
	os_become_orig();
	fp = glue_fopen(filename->str_text, "w");
	if (!fp)
		fatal("create %s", filename->str_text);
	glue_fclose(fp);
	os_edit(filename);
	result = read_whole_file(filename->str_text);
	os_unlink(filename);
	os_become_undo();
	return result;
}


string_ty *
os_edit_filename(at_home)
	int		at_home;
{
	static int	num;
	string_ty	*buffer;
	string_ty	*result;
	char		*dir;
	int		name_max;

	if (at_home)
	{
		dir = getenv("HOME");
		if (!dir || dir[0] != '/')
			dir = "/tmp";
	}
	else
		dir = "/tmp";
	name_max = 14;
	buffer = str_format("-%d-%d", getpid(), ++num);
	result =
		str_format
		(
			"%s/%.*s%S",
			dir,
			(int)(name_max - buffer->str_length),
			option_progname_get(),
			buffer
		);
	str_free(buffer);
	return result;
}


int
os_pathconf_name_max(path)
	string_ty	*path;
{
	long		result;
	char		*p;

	p = path->str_text;
	os_become_must_be_active();
#ifdef _PC_NAME_MAX
	result = glue_pathconf(p, _PC_NAME_MAX);
	if (result < 0 && (errno == EINVAL || errno == ENOSYS))
	{
		/*
		 * Probably NFS/2 mounted (NFS/3 added pathconf), so
		 * we will *guess* it's the same as the root filesystem.
		 * Default to 14 if root is also NFS mounted.
		 */
		p = "/";
		result = glue_pathconf(p, _PC_NAME_MAX);
		if (result < 0 && (errno == EINVAL || errno == ENOSYS))
		{
#ifdef HAVE_LONG_FILE_NAMES
			result = 255;
#else
			result = 14;
#endif
		}
	}
	if (result < 0)
		nfatal("pathconf(\"%s\", {NAME_MAX})", p);
#if INT_MAX < LONG_MAX
	if (result > INT_MAX)
		result = INT_MAX;
#endif
#else

	/*
	 * system does not have pathconf
	 */
#ifdef HAVE_LONG_FILE_NAMES
	result = 255;
#else
	result = 14;
#endif
#endif
	return result;
}


void
os_symlink(src, dst)
	string_ty	*src;
	string_ty	*dst;
{
	trace(("os_symlink()\n{\n"/*}*/));
	os_become_must_be_active();
#ifdef S_IFLNK
	if (glue_symlink(src->str_text, dst->str_text))
		nfatal("symlink(\"%s\", \"%s\")", src->str_text, dst->str_text);
#else
	fatal
	(
		"symlink(\"%s\", \"%s\"): symbolic links not available",
		src->str_text,
		dst->str_text
	);
#endif
	trace((/*{*/"}\n"));
}


string_ty *
os_readlink(path)
	string_ty	*path;
{
	int		nbytes;
	string_ty	*result;
	char		buffer[2000];

	trace(("os_readlink(\"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
#ifdef S_IFLNK
	nbytes = glue_readlink(path->str_text, buffer, sizeof(buffer));
	if (nbytes < 0)
		nfatal("readlink(\"%s\")", path->str_text);
	if (nbytes == 0)
		fatal("readlink(\"%s\") returned \"\"", path->str_text);
	result = str_n_from_c(buffer, nbytes);
#else
	fatal
	(
		"readlink(\"%s\"): symbolic links not available",
		src->str_text,
		dst->str_text
	);
	result = str_copy(path);
#endif
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


int
os_symlink_query(path)
	string_ty	*path;
{
	int		result;
	struct stat	st;

	trace(("os_symlink_query(\"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
#ifdef S_IFLNK
	result =
		(
			!glue_lstat(path->str_text, &st)
		&&
			(st.st_mode & S_IFMT) == S_IFLNK
		);
#else
	result = 0;
#endif
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
os_junkfile(path, mode)
	string_ty	*path;
	int		mode;
{
	os_become_must_be_active();
	if (glue_junkfile(path->str_text))
		nfatal("fill %s with gibberish", path->str_text);
	if (glue_chmod(path->str_text, mode))
		nfatal("chmod(\"%s\", 0%o)", path->str_text, mode);
}


/*
 * NAME
 *	os_throttle
 *
 * SYNOPSIS
 *	void os_throttle(void);
 *
 * DESCRIPTION
 *	This unlikely function is used to slow aegis down.  it is
 *	primarily used for aegis' own tests, to ensure that the time
 *	stamps are kosher even on ultra-fast machines.  It is also
 *	useful in shell scripts, e.g. automatic integration queue
 *	handling.
 */

void
os_throttle()
{
	static int	nsecs;

	if (nsecs == 0)
	{
		string_ty	*s1;
		string_ty	*s2;
		char		*cp;

		s1 = str_format("%s_THROTTLE", option_progname_get());
		s2 = str_upcase(s1);
		str_free(s1);
		cp = getenv(s2->str_text);
		str_free(s2);
		if (!cp)
			nsecs = -1;
		else
		{
			nsecs = atoi(cp);
			if (nsecs <= 0)
				nsecs = -1;
			else if (nsecs > 5)
				nsecs = 5;
		}
	}
	if (nsecs > 0)
		sleep(nsecs);
}
