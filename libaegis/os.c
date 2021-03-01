/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998 Peter Miller;
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
 * MANIFEST: wrappers around operating system functions
 */

#include <ac/stdlib.h>
#include <errno.h>
#include <ac/stdio.h>
#include <ac/string.h>
#include <ac/limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <ac/unistd.h>
#include <utime.h>
#include <ac/pwd.h>
#include <ac/grp.h>
#include <ac/fcntl.h>

#include <arglex2.h>
#include <dir.h>
#include <error.h>
#include <file.h>
#include <fopen_nfs.h>
#include <glue.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <progname.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <str_list.h>

#define MAX_CMD_RPT 36


static	int	become_orig_uid;
static	int	become_orig_gid;
static	int	become_orig_umask;
static	int	become_inited;
static	int	become_testing;
static	int	become_active;
static	int	become_active_uid;
static	int	become_active_gid;
static	int	become_active_umask;
static	int	interrupted;


static RETSIGTYPE interrupt _((int));

static RETSIGTYPE
interrupt(n)
	int	n;
{
	sub_context_ty	*scp;

	signal(n, SIG_IGN);
	if (interrupted)
		return;
	interrupted = 1;
	scp = sub_context_new();
	sub_var_set(scp, "Signal", "%s", strsignal(n));
	fatal_intl(scp, i18n("interrupted by $signal"));
	/* NOTREACHED */
	sub_context_delete(scp);
}


void
os_interrupt_register()
{
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		signal(SIGHUP, interrupt);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, interrupt);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, interrupt);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, interrupt);
}


void
os_interrupt_ignore()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
}


void
os_interrupt_cope()
{
}


int
os_interrupt_has_occurred()
{
	return (interrupted != 0);
}


int
os_waitpid_status(child, cmd)
	int		child;
	char		*cmd;
{
	sub_context_ty	*scp;
	int		result;
	RETSIGTYPE	(*hold)_((int));
	int		a, b, c;
	int		status;

	trace(("os_waitpid_status(child = %d, cmd = \"%s\")\n{\n"/*}*/, child, cmd));
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
		scp = sub_context_new();
		sub_var_set(scp, "Command", "%s", cmd);
		fatal_intl(scp, i18n("command \"$command\" stopped"));
		/* NOTREACHED */

	case 0:
		/* normal termination */
		result = a;
		break;

	default:
		/*
		 * process dies from unhandled condition
		 */
		scp = sub_context_new();
		sub_var_set(scp, "Command", "%s", cmd);
		sub_var_set(scp, "Signal", "%s", strsignal(c));
		if (!b)
			fatal_intl(scp, i18n("command \"$command\" terminated by $signal"));
		else
			fatal_intl(scp, i18n("command \"$command\" terminated by $signal (core dumped)"));
		/* NOTREACHED */
	}
	if (hold != SIG_IGN)
		signal(SIGINT, hold);
	os_interrupt_cope();
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
		if (errno != ENOENT && errno != ENOTDIR)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set(scp, "File_Name", "%S", path);
			fatal_intl(scp, i18n("stat $filename: $errno"));
			/* NOTREACHED */
		}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Argument", "0%o", mode);
		fatal_intl(scp, i18n("mkdir(\"$filename\", $arg): $errno"));
		/* NOTREACHED */
	}

	/*
	 * There could be Berkeley semantics about the group
	 * of the newly created directory, so make sure it is
	 * the one intended (egid).
	 */
	os_become_query(&uid, &gid, &um);
	if (glue_chown(path->str_text, uid, gid))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Argument", "%d", gid);
		fatal_intl(scp, i18n("chgrp(\"$filename\", $arg): $errno"));
		/* NOTREACHED */
	}

	/*
	 * The set-group-id bit is ignored by a Berkeley semantics mkdir
	 * (and it would be nuked the the chgrp, anyway)
	 * so set it explicitly.
	 */
	mode &= ~um;
	if (glue_chmod(path->str_text, mode))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Argument", "0$o", mode);
		fatal_intl(scp, i18n("chmod(\"$filename\", $arg): $errno"));
		/* NOTREACHED */
	}
	trace((/*{*/"}\n"));
}


void
os_rmdir(path)
	string_ty	*path;
{
	trace(("os_rmdir(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_rmdir(path->str_text) && errno != ENOENT)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("rmdir $filename: $errno"));
		/* NOTREACHED */
	}
	trace((/*{*/"}\n"));
}


void
os_rmdir_errok(path)
	string_ty	*path;
{
	trace(("os_rmdir_errok(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_rmdir(path->str_text) && errno != ENOENT)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		error_intl(scp, i18n("warning: rmdir $filename: $errno"));
		sub_context_delete(scp);
	}
	trace((/*{*/"}\n"));
}


void
os_rmdir_bg(path)
	string_ty	*path;
{
	trace(("os_rmdir_bg(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_rmdir_bg(path->str_text))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		error_intl(scp, i18n("warning: rmdir $filename: $errno"));
		sub_context_delete(scp);
	}
	trace((/*{*/"}\n"));
}


void
os_rmdir_tree(path)
	string_ty	*path;
{
	trace(("os_rmdir_tree(path = \"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_rmdir_tree(path->str_text))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		error_intl(scp, i18n("warning: rmdir $filename: $errno"));
		sub_context_delete(scp);
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name1", "%S", a);
		sub_var_set(scp, "File_Name2", "%S", b);
		fatal_intl(scp, i18n("rename(\"$filename1\", \"$filename2\"): $errno"));
		/* NOTREACHED */
	}
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
	if (oret && errno == ENOENT)
	{
		/* Don't complain if it's not there. */
		trace((/*{*/"}\n"));
		return;
	}
	if (oret)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("stat $filename: $errno"));
		/* NOTREACHED */
	}
	if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_setx(scp, EISDIR);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("unlink $filename: $errno"));
		/* NOTREACHED */
	}
	if (glue_unlink(path->str_text))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("unlink $filename: $errno"));
		/* NOTREACHED */
	}
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
	if (oret && errno == ENOENT)
	{
		/* Don't complaint if it's not there. */
		trace((/*{*/"}\n"));
		return;
	}
	if (oret)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		if (errno != ENOENT && errno != ENOTDIR)
			fatal_intl(scp, i18n("stat $filename: $errno"));
		error_intl(scp, i18n("warning: stat $filename: $errno"));
		sub_context_delete(scp);
	}
	else if ((st.st_mode & S_IFMT) == S_IFDIR)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_setx(scp, EISDIR);
		sub_var_set(scp, "File_Name", "%S", path);
		error_intl(scp, i18n("warning: unlink $filename: $errno"));
		sub_context_delete(scp);
	}
	else if (glue_unlink(path->str_text))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		error_intl(scp, i18n("warning: unlink $filename: $errno"));
		sub_context_delete(scp);
	}
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
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_errno_set(scp);
			fatal_intl(scp, i18n("getcwd: $errno"));
			/* NOTREACHED */
		}
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
	if (!path)
		path = os_curdir();
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
				&& 
					errno != ENOTDIR
				)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_errno_set(scp);
					sub_var_set(scp, "File_Name", "%S", s);
					fatal_intl(scp, i18n("readlink $filename: $errno"));
					/* NOTREACHED */
				}
				str_free(s);
			}
			else
			{
				string_ty	*newpath;
	
				if (nbytes == 0)
				{
					pointer[0] = '.';
					nbytes = 1;
				}
				if (++loop > 1000)
				{
					sub_context_ty	*scp;

					scp = sub_context_new();
					sub_errno_setx(scp, ELOOP);
					sub_var_set(scp, "File_Name", "%S", s);
					fatal_intl(scp, i18n("readlink $filename: $errno"));
					/* NOTREACHED */
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("chdir $filename: $errno"));
		/* NOTREACHED */
	}
}


void
os_setuid(uid)
	int		uid;
{
	os_become_must_not_be_active();
	if (setuid(uid))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "Argument", "%d", uid);
		fatal_intl(scp, i18n("setuid $arg: $errno"));
		/* NOTREACHED */
	}
}


void
os_setgid(gid)
	int		gid;
{
	os_become_must_not_be_active();
	if (become_testing < 0)
		return;
	if (setgid(gid))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "Argument", "%d", gid);
		if (become_testing)
		{
			/*
			 * don't use os_testing_mode(),
			 * it could mess with errno
			 */
			error_intl(scp, i18n("warning: setgid $arg: $errno"));
		}
		else
			fatal_intl(scp, i18n("setgid $arg: $errno"));
		sub_context_delete(scp);
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
		sub_context_ty	*scp;
		string_ty	*s;

		if (cmd->str_length > MAX_CMD_RPT)
			s = str_format("%.*S...", MAX_CMD_RPT - 3, cmd);
		else
			s = str_copy(cmd);
		scp = sub_context_new();
		sub_var_set(scp, "Command", "%S", s);
		sub_var_set(scp, "Number", "%d", result);
		fatal_intl(scp, i18n("command \"$command\" exit status $number"));
		/* NOTREACHED */
		sub_context_delete(scp);
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
		sub_context_ty	*scp;

		str_free(last_dir);
		last_dir = rdir;
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", dir);
		error_intl(scp, i18n("cd $filename"));
		sub_context_delete(scp);
	}
	else
		str_free(rdir);

	if (last_uid != uid || last_gid != gid)
	{
		sub_context_ty	*scp;
		struct passwd	*pw;
		struct group	*gr;

		scp = sub_context_new();
		last_uid = uid;
		pw = getpwuid(uid);
		if (pw)
			sub_var_set(scp, "Name1", "\"%s\"", pw->pw_name);
		else
			sub_var_set(scp, "Name1", "%d", uid);

		last_gid = gid;
		gr = getgrgid(gid);
		if (gr)
			sub_var_set(scp, "Name2", "\"%s\"", gr->gr_name);
		else
			sub_var_set(scp, "Name2", "%d", gid);
		error_intl(scp, i18n("user $name1, group $name2"));
		sub_context_delete(scp);
	}
}


int
os_execute_retcode(cmd, flags, dir)
	string_ty	*cmd;
	int		flags;
	string_ty	*dir;
{
	sub_context_ty	*scp;
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
		if (dir)
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
				fatal_raw("dup gave %d, not 0 (bug)", n);
			if (close(pfd[0]) || close(pfd[1]))
				nfatal("close pipe ends");
		}

		/*
		 * let the log file (user) know what we did
		 */
		scp = sub_context_new();
		sub_var_set(scp, "Message", "%S", cmd);
		error_intl(scp, "$message");
		sub_context_delete(scp);

		/*
		 * invoke the command through sh(1)
		 */
		shell = os_shell();
		execl(shell, shell, "-ec", cmd->str_text, (char *)0);
		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", shell);
		fatal_intl(scp, i18n("exec \"$filename\": $errno"));
		/* NOTREACHED */
	}
	if (result && (flags & OS_EXEC_FLAG_ERROK))
	{
		scp = sub_context_new();
		sub_var_set(scp, "Command", "%S", cmd);
		sub_var_set(scp, "Number", "%d", result);
		error_intl(scp, i18n("warning: command \"$command\" exit status $number"));
		sub_context_delete(scp);
		result = 0;
	}
	str_free(cmd2);
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
os_mtime_range(path, oldest_p, newest_p)
	string_ty	*path;
	time_t		*oldest_p;
	time_t		*newest_p;
{
	struct stat	st;
	int		oret;

	trace(("os_mtime_range(path = %08lX)\n{\n"/*}*/, path));
	os_become_must_be_active();
	trace_string(path->str_text);

#ifdef S_IFLNK
	oret = glue_lstat(path->str_text, &st);
#else
	oret = glue_stat(path->str_text, &st);
#endif
	if (oret)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("stat $filename: $errno"));
		/* NOTREACHED */
	}

	/*
	 * Return the last modified time.
	 * They may try to fake us out by using utime,
	 * so check the inode change time, too.
	 */
	*newest_p = (st.st_ctime > st.st_mtime ? st.st_ctime : st.st_mtime);
	*oldest_p = (st.st_ctime < st.st_mtime ? st.st_ctime : st.st_mtime);
	trace((/*{*/"}\n"));
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("utime $filename: $errno"));
		/* NOTREACHED */
	}
	trace((/*{*/"}\n"));
}


void
os_mtime_set_errok(path, when)
	string_ty	*path;
	time_t		when;
{
	struct utimbuf	utb;

	trace(("os_mtime_set_errok(path = %08lX, when = %ld)\n{\n"/*}*/, path, when));
	os_become_must_be_active();
	trace_string(path->str_text);
	trace(("when = %s", ctime(&when)));
	utb.actime = when;
	utb.modtime = when;
	if (glue_utime(path->str_text, &utb))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		error_intl(scp, i18n("warning: utime $filename: $errno"));
		sub_context_delete(scp);
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("stat $filename: $errno"));
		/* NOTREACHED */
	}
	if ((st.st_mode & 07777) != mode)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Number1", "%5.5o", st.st_mode & 07777);
		sub_var_set(scp, "Number2", "%5.5o", mode);
		error_intl
		(
			scp,
			i18n("$filename: mode is $number1, should be $number2")
		);
		sub_context_delete(scp);
		++nerrs;
	}
	if (become_testing > 0)
	{
		/*
		 * This is the test performed by aegis
		 * in testing (not set-uid-root) mode.
		 */
		if (st.st_uid != geteuid())
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", path);
			sub_var_set(scp, "Number1", "%d", st.st_uid);
			sub_var_set(scp, "Number2", "%d", geteuid());
			error_intl
			(
				scp,
			i18n("$filename: owner is $number1, should be $number2")
			);
			sub_context_delete(scp);
			++nerrs;
		}
	}
	else if (become_testing == 0)
	{
		/*
		 * This is the test performed by aegis
		 * when is set-uid-root mode.
		 */
		if (st.st_uid != uid)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", path);
			sub_var_set(scp, "Number1", "%d", st.st_uid);
			sub_var_set(scp, "Number2", "%d", uid);
			error_intl
			(
				scp,
			i18n("$filename: owner is $number1, should be $number2")
			);
			sub_context_delete(scp);
			++nerrs;
		}
		if (gid >= 0 && st.st_gid != gid)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "File_Name", "%S", path);
			sub_var_set(scp, "Number1", "%d", st.st_gid);
			sub_var_set(scp, "Number2", "%d", gid);
			error_intl
			(
				scp,
			i18n("$filename: group is $number1, should be $number2")
			);
			sub_context_delete(scp);
			++nerrs;
		}
	}
	else
	{
		/*
		 * No test is performed by aereport, aefind, etc.
		 * This is because testing mode and non-testing mode
		 * are indistinguishable.
		 */
	}
	if (nerrs)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("$filename: has been tampered with (fatal)"));
		/* NOTREACHED */
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Argument", "%5.5o", mode);
		fatal_intl(scp, i18n("chmod(\"$filename\", $arg): $errno"));
		/* NOTREACHED */
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("stat $filename: $errno"));
		/* NOTREACHED */
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Argument", "%5.5o", mode);
		error_intl(scp, i18n("warning: chmod(\"$filename\", $arg): $errno"));
		sub_context_delete(scp);
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name1", "%S", from);
		sub_var_set(scp, "File_Name2", "%S", to);
		fatal_intl(scp, i18n("link(\"$filename1\", \"$filename2\"): $errno"));
		/* NOTREACHED */
	}
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


void
os_become_init()
{
	assert(!become_inited);
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


void
os_become_init_mortal()
{
	assert(!become_inited);
	become_active_umask = DEFAULT_UMASK;
	become_orig_umask = umask(DEFAULT_UMASK);
	become_orig_uid = getuid();
	become_orig_gid = getgid();
	become_testing = -1;
	become_active_uid = become_orig_uid;
	become_active_gid = become_orig_gid;
	become_inited = 1;
}


int
os_testing_mode()
{
	static int warned;

	assert(become_inited);
	if (become_testing <= 0)
		return 0;
	if (!warned)
	{
		sub_context_ty	*scp;

		warned = 1;
		scp = sub_context_new();
		verbose_intl(scp, i18n("warning: test mode"));
		sub_context_delete(scp);
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
		fatal_raw("multiple user permissions set (bug)");
	become_active = 1;
	if (os_testing_mode())
	{
		become_active_umask = um;
		umask(um);
	}
	else if (become_testing < 0)
	{
		/* do nothing if we are mortal */
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
		fatal_raw("%s: %d: user permissions not set (bug)", file, line);
}


void
os_become_must_not_be_active_gizzards(file, line)
	char	*file;
	int	line;
{
	if (os_become_active())
	{
		fatal_raw
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
	 *	The process group the terminal is in is the forground.
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
	 * There are reports that Ksh does something else,
	 * and this function is frequently wrong.
	 * Anybody out there use Ksh and know what to do?
	 */

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
os_edit(filename, et)
	string_ty	*filename;
	edit_ty		et;
{
	string_ty	*cmd;
	string_ty	*cwd;
	char		*editor;

	/*
	 * find the editor to use
	 */
	if (et == edit_background)
	{
		editor = getenv("EDITOR");
		if (!editor || !*editor)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "EDITOR");
			fatal_intl(scp, i18n("environment variable $name not set"));
			/* NOTREACHED */
		}
	}
	else
	{
		editor = getenv("VISUAL");
		if (!editor || !*editor)
		{
			editor = getenv("EDITOR");
			if (editor && *editor)
			{
				char		*cp;

				cp = strrchr(editor, '/');
				if (!cp)
					cp = editor;
				if (0 == strcmp(cp, "ed"))
					et = edit_background;
			}
			else
				editor = "vi";
		}
	}

	/*
	 * make sure we are in a position to edit
	 */
	if (et != edit_background && os_background())
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Name", "%s", arglex_token_name(arglex_token_edit));
		fatal_intl(scp, i18n("may not use $name in the background"));
		/* NOTREACHED */
	}

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
os_edit_new(et)
	edit_ty		et;
{
	string_ty	*filename;
	string_ty	*result;
	FILE		*fp;

	filename = os_edit_filename(0);
	os_become_orig();
	fp = fopen_with_stale_nfs_retry(filename->str_text, "w");
	if (!fp)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%s", filename);
		fatal_intl(scp, i18n("open $filename: $errno"));
		sub_context_delete(scp);
	}
	glue_fclose(fp);
	os_edit(filename, et);
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
	{
		dir = getenv("TMPDIR");
		if (!dir || dir[0] != '/')
			dir = "/tmp";
	}
	name_max = 14;
	buffer = str_format("-%d-%d", getpid(), ++num);
	result =
		str_format
		(
			"%s/%.*s%S",
			dir,
			(int)(name_max - buffer->str_length),
			progname_get(),
			buffer
		);
	str_free(buffer);
	return result;
}


static long pathconf_inner _((char *, int));

static long
pathconf_inner(path, arg)
	char		*path;
	int		arg;
{
#ifdef HAVE_PATHCONF
	long		result;

	errno = EINVAL; /* IRIX 5.2 fails to set on errors */
	result = glue_pathconf(path, arg);
	if (result < 0)
	{
		switch (errno)
		{
		case ENOSYS: /* lotsa systems say this for EINVAL */
#ifdef EOPNOTSUPP
		case EOPNOTSUPP: /* HPUX says this for EINVAL */
#endif
			errno = EINVAL;
			break;
		}
	}
	else
	{
#if INT_MAX < LONG_MAX
		if (result > INT_MAX)
			result = INT_MAX;
#endif
	}
	return result;
#else
	/*
	 * This system does not have the pathconf system call.
	 */
	errno = ENOSYS;
	return -1;
#endif
}


static long pathconf_wrapper _((char *, int, long));

static long
pathconf_wrapper(path, arg, default_value)
	char		*path;
	int		arg;
	long		default_value;
{
#ifdef HAVE_PATHCONF
	long		result;

	result = pathconf_inner(path, arg);
	if (result < 0 && errno == EINVAL)
	{
		/*
		 * Probably NFS/2 mounted (NFS/3 added pathconf), so
		 * we will *guess* it's the same as the root filesystem.
		 * Default if root is also NFS mounted.
		 */
		result = pathconf_inner("/", arg);
		if (result < 0 && errno == EINVAL)
			result = default_value;
	}
	return result;
#else
	/*
	 * This system does not have the pathconf system call.
	 */
	return default_value;
#endif
}


int
os_pathconf_path_max(path)
	string_ty	*path;
{
	long		result;

	os_become_must_be_active();
	result = 1024;
	result = pathconf_wrapper(path->str_text, _PC_PATH_MAX, result);
	if (result < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("pathconf(\"$filename\", {PATH_MAX}): $errno"));
		/* NOTREACHED */
	}
	return result;
}


int
os_pathconf_name_max(path)
	string_ty	*path;
{
	long		result;

	os_become_must_be_active();
#ifdef HAVE_LONG_FILE_NAMES
	result = 255;
#else
	result = 14;
#endif
	result = pathconf_wrapper(path->str_text, _PC_NAME_MAX, result);
	if (result < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("pathconf(\"$filename\", {NAME_MAX}): $errno"));
		/* NOTREACHED */
	}
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name1", "%S", src);
		sub_var_set(scp, "File_Name2", "%S", dst);
		fatal_intl(scp, i18n("symlink(\"$filename1\", \"$filename2\"): $errno"));
		sub_context_delete(scp);
	}
#else
	fatal_raw
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("readlink $filename: $errno"));
		sub_context_delete(scp);
	}
	if (nbytes == 0)
	{
		buffer[0] = '.';
		nbytes = 1;
	}
	result = str_n_from_c(buffer, nbytes);
#else
	fatal_raw
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
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("junkfile $filename: $errno"));
		sub_context_delete(scp);
	}
	if (glue_chmod(path->str_text, mode))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		sub_var_set(scp, "Argument", "%5.5o", mode);
		fatal_intl(scp, i18n("chmod(\"$filename\", $arg): $errno"));
		/* NOTREACHED */
	}
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
		char		*cp;

		cp = getenv("AEGIS_THROTTLE");
		if (!cp)
			nsecs = -1;
		else
		{
			nsecs = atoi(cp);
			if (nsecs <= 0)
				nsecs = 1;
			else if (nsecs > 5)
				nsecs = 5;
		}
	}
	if (nsecs > 0)
	{
		sync();
#ifdef DEBUG
		error_raw("throttling %d second%s", nsecs, (nsecs == 1 ? "" : "s"));
#endif
		sleep(nsecs);
	}
}


void
os_owner_query(path, uid_p, gid_p)
	string_ty	*path;
	int		*uid_p;
	int		*gid_p;
{
	struct stat	st;

	trace(("os_owner_query(\"%s\")\n{\n"/*}*/, path->str_text));
	os_become_must_be_active();
	if (glue_stat(path->str_text, &st) < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("stat $filename: $errno"));
		/* NOTREACHED */
	}
	if (uid_p)
		*uid_p = st.st_uid;
	if (gid_p)
		*gid_p = st.st_gid;
	trace((/*{*/"}\n"));
}


string_ty *
os_fingerprint(path)
	string_ty	*path;
{
	int		nbytes;
	char		buf[1000];

	os_become_must_be_active();
	nbytes = file_fingerprint(path->str_text, buf, sizeof(buf));
	if (nbytes < 0)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_errno_set(scp);
		sub_var_set(scp, "File_Name", "%S", path);
		fatal_intl(scp, i18n("fingerprint $filename: $errno"));
		/* NOTREACHED */
	}
	return str_n_from_c(buf, nbytes);
}
