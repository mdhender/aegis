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
 * MANIFEST: functions to report errors
 */

#include <ctype.h>
#include <errno.h>
#include <ac/stddef.h>
#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <grp.h>
#include <pwd.h>
#include <ac/unistd.h>
#include <ac/stdarg.h>

#include <arglex.h>
#include <error.h>
#include <mprintf.h>
#include <option.h>


static void error_get_id _((int *, int *));

static void
error_get_id(uid, gid)
	int	*uid;
	int	*gid;
{
	*uid = geteuid();
	*gid = getegid();
}


static	error_id_ty errid = error_get_id;


void
error_set_id_func(f)
	error_id_ty	f;
{
	if (f)
		errid = f;
	else
		errid = error_get_id;
}


/*
 * NAME
 *	wrap - wrap s string over lines
 *
 * SYNOPSIS
 *	void wrap(char *);
 *
 * DESCRIPTION
 *	The wrap function is used to print error messages onto stderr
 *	wrapping ling lines.
 *
 * CAVEATS
 *	Line length is assumed to be 80 characters.
 */

static void wrap _((char *));

static void
wrap(s)
	char	*s;
{
	char	*progname;
	static char escapes[] = "\rr\nn\ff\bb\tt";
	int	page_width;
	char	tmp[MAX_PAGE_WIDTH + 2];
	int	first_line;
	char	*tp;

	if (fflush(stdout) || ferror(stdout))
		nfatal("(stdout)");
	/* don't use last column, many terminals are dumb */
	page_width = option_page_width_get() - 1;
	progname = option_progname_get();
	first_line = 1;
	while (*s)
	{
		char	*ep;
		int	ocol;

		/*
		 * Work out how many characters fit on the line.
		 */
		if (first_line)
			ocol = strlen(progname) + 2;
		else
			ocol = 8;
		for (ep = s; *ep; ++ep)
		{
			int	cw;
			int	c;

			c = (unsigned char)*ep;
			if (isprint(c))
				cw = 1 + (c == '\\');
			else
				cw = (strchr(escapes, c) ? 2 : 4);
			if (ocol + cw > page_width)
				break;
			ocol += cw;
		}

		/*
		 * see if there is a better place to break the line
		 */
		if (*ep && *ep != ' ')
		{
			char	*mp;

			for (mp = ep; mp > s; --mp)
			{
				if (strchr(" /", mp[-1]))
				{
					ep = mp;
					break;
				}
			}
		}

		/*
		 * ignore trailing blanks
		 */
		while (ep > s && ep[-1] == ' ')
			ep--;

		/*
		 * print the line
		 */
		if (first_line)
			sprintf(tmp, "%s: ", progname);
		else
			strcpy(tmp, "\t");
		tp = tmp + strlen(tmp);
		while (s < ep)
		{
			int	c;

			c = (unsigned char)*s++;
			if (isprint(c))
			{
				if (c == '\\')
					*tp++ = '\\';
				*tp++ = c;
			}
			else
			{
				char	*esc;

				esc = strchr(escapes, c);
				if (esc)
				{
					*tp++ = '\\';
					*tp++ = esc[1];
				}
				else
				{
					sprintf(tp, "\\%3.3o", c);
					tp += strlen(tp);
				}
			}
		}
		*tp++ = '\n';
		*tp = 0;
		fputs(tmp, stderr);
		if (ferror(stderr))
			break;

		/*
		 * skip leading spaces for subsequent lines
		 */
		while (*s == ' ')
			s++;
		first_line = 0;
	}
	if (fflush(stderr) || ferror(stderr))
		nfatal("(stderr)");
}


static void double_jeopardy _((void));

static void
double_jeopardy()
{
	char	buffer[200];

	sprintf
	(
		buffer,
		"while attempting to construct an error message: %s (fatal)",
		strerror(errno)
	);
	wrap(buffer);
	quit(1);
}


static char *copy_string _((char *));

static char *
copy_string(s)
	char		*s;
{
	char		*cp;

	errno = 0;
	cp = malloc(strlen(s) + 1);
	if (!cp)
	{
		if (!errno)
			errno = ENOMEM;
		double_jeopardy();
	}
	strcpy(cp, s);
	return cp;
}


static char *id _((void));

static char *
id()
{
	int		uid;
	struct passwd	*pw;
	char		uidn[20];
	int		gid;
	struct group	*gr;
	char		gidn[20];
	char		buffer[100];

	errid(&uid, &gid);
	pw = getpwuid(uid);
	if (pw)
		sprintf(uidn, "user \"%.8s\"", pw->pw_name);
	else
		sprintf(uidn, "uid %d", uid);

	gr = getgrgid(gid);
	if (gr)
		sprintf(gidn, "group \"%.8s\"", gr->gr_name);
	else
		sprintf(gidn, "gid %d", gid);

	sprintf(buffer, " [%s, %s]", uidn, gidn);
	return copy_string(buffer);
}


/*
 *  NAME
 *	error - place a message on the error stream
 *
 *  SYNOPSIS
 *	void error(char *fmt, ...);
 *
 *  DESCRIPTION
 *	Error places a message on the error output stream.
 *	The first argument is a printf-like format string,
 *	optionally followed by other arguments.
 *	The message will be prefixed by the program name and a colon,
 *	and will be terminated with a newline, automatically.
 *
 *  CAVEAT
 *	Things like "error(filename)" blow up if the filename
 *	contains a '%' character.
 */

/*VARARGS1*/
void
error(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	char		*buffer;

	sva_init(ap, fmt);
	buffer = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!buffer)
		double_jeopardy();
	wrap(buffer);
}


/*
 *  NAME
 *	nerror - place a system fault message on the error stream
 *
 *  SYNOPSIS
 *	void nerror(char *fmt, ...);
 *
 *  DESCRIPTION
 *	Nerror places a message on the error output stream.
 *	The first argument is a printf-like format string,
 *	optionally followed by other arguments.
 *	The message will be prefixed by the program name and a colon,
 *	and will be terminated with a text description of the error
 *	indicated by the 'errno' global variable, automatically.
 *
 *  CAVEAT
 *	Things like "nerror(filename)" blow up if the filename
 *	contains a '%' character.
 */

/*VARARGS1*/
void
nerror(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	char		*s1;
	char		*s2;
	va_list		ap;
	int		n;

	n = errno;
	sva_init(ap, fmt);
	s1 = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!s1)
		double_jeopardy();
	s1 = copy_string(s1);
	if (n == EPERM || n == EACCES)
		s2 = id();
	else
		s2 = copy_string("");
	error("%s: %s%s", s1, strerror(n), s2);
	free(s1);
	free(s2);
}


/*
 *  NAME
 *	nfatal - place a system fault message on the error stream and exit
 *
 *  SYNOPSIS
 *	void nfatal(char *fmt, ...);
 *
 *  DESCRIPTION
 *	Nfatal places a message on the error output stream and exits.
 *	The first argument is a printf-like format string,
 *	optionally followed by other arguments.
 *	The message will be prefixed by the program name and a colon,
 *	and will be terminated with a text description of the error
 *	indicated by the 'errno' global variable, automatically.
 *
 *  CAVEAT
 *	Things like "nfatal(filename)" blow up if the filename
 *	contains a '%' character.
 *
 *	This function does NOT return.
 */

/*VARARGS1*/
void
nfatal(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	char		*s1;
	char		*s2;
	va_list		ap;
	int		n;

	n = errno;
	sva_init(ap, fmt);
	s1 = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!s1)
		double_jeopardy();
	s1 = copy_string(s1);

	if (n == EPERM || n == EACCES)
		s2 = id();
	else
		s2 = "";

	fatal("%s: %s%s", s1, strerror(n), s2);
}


/*
 *  NAME
 *	fatal - place a message on the error stream and exit
 *
 *  SYNOPSIS
 *	void fatal(char *fmt, ...);
 *
 *  DESCRIPTION
 *	Fatal places a message on the error output stream and exits.
 *	The first argument is a printf-like format string,
 *	optionally followed by other arguments.
 *	The message will be prefixed by the program name and a colon,
 *	and will be terminated with a newline, automatically.
 *
 *  CAVEAT
 *	Things like "error(filename)" blow up if the filename
 *	contains a '%' character.
 *
 *	This function does NOT return.
 */

/*VARARGS1*/
void
fatal(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	char		*buffer;

	sva_init(ap, fmt);
	buffer = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!buffer)
		double_jeopardy();
	wrap(buffer);
	quit(1);
}


/*
 *  NAME
 *	assert - make an assertion
 *
 *  SYNOPSIS
 *	void assert(int condition);
 *
 *  DESCRIPTION
 *	Assert is a handy tool for a programmer to guarantee the internal
 *	consistency of their program. If "-DDEBUG" is specified on
 *	the compiler's command line, then assert will generate code to verify
 *	the assertios made. If no DEBUG is defined, assertions will generate
 *	no code.
 *
 *  CAVEAT
 *	If the assertion fails, a fatal diagnostic is issued.
 *
 *	The #define's which control the compilation may be found in "error.h".
 *
 */

int
assert_failed(s, file, line)
	char	*s;
	char	*file;
	int	line;
{
	error("%s: %d: assertion \"%s\" failed (bug)", file, line, s);
	abort();
	exit(1); /* incase abort() comes back */
	return 0; /* to silence lint */
}


/*
 *  NAME
 *	verbose - place a verbose message on the error stream
 *
 *  SYNOPSIS
 *	void verbose(char *fmt, ...);
 *
 *  DESCRIPTION
 *	The verbose function places a verbose message on the error stream.
 *	The first argument is a printf-like format string,
 *	optionally followed by other arguments.
 *	The message will be prefixed by the program name and a colon,
 *	and will be terminated with a newline, automatically.
 *
 *  CAVEAT
 *	Things like "verbose(filename)" blow up if the filename
 *	contains a '%' character.
 */

/*VARARGS1*/
void
verbose(fmt sva_last)
	char		*fmt;
	sva_last_decl
{
	va_list		ap;
	char		*buffer;

	sva_init(ap, fmt);
	buffer = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!buffer)
		double_jeopardy();
	if (option_verbose_get())
		wrap(buffer);
}


/*
 *  NAME
 *      signal_name - find it
 *
 *  SYNOPSIS
 *      char *signal_name(int n);
 *
 *  DESCRIPTION
 *      The signal_name function is used to find the name of a signal from its
 *      number.
 *
 *  RETURNS
 *      char *: pointer to the signal name.
 *
 *  CAVEAT
 *      The signal name may not be written on.  Subsequent calls may alter the
 *      area pointed to.
 */

char *
signal_name(n)
	int	n;
{
	static char buffer[16];

	switch (n)
	{
#ifdef SIGHUP
	case SIGHUP:
		return "hang up [SIGHUP]";
#endif /* SIGHUP */

#ifdef SIGINT
	case SIGINT:
		return "user interrupt [SIGINT]";
#endif /* SIGINT */

#ifdef SIGQUIT
	case SIGQUIT:
		return "user quit [SIGQUIT]";
#endif /* SIGQUIT */

#ifdef SIGILL
	case SIGILL:
		return "illegal instruction [SIGILL]";
#endif /* SIGILL */

#ifdef SIGTRAP
	case SIGTRAP:
		return "trace trap [SIGTRAP]";
#endif /* SIGTRAP */

#ifdef SIGIOT
	case SIGIOT:
		return "abort [SIGIOT]";
#endif /* SIGIOT */

#ifdef SIGEMT
	case SIGEMT:
		return "EMT instruction [SIGEMT]";
#endif /* SIGEMT */

#ifdef SIGFPE
	case SIGFPE:
		return "floating point exception [SIGFPE]";
#endif /* SIGFPE */

#ifdef SIGKILL
	case SIGKILL:
		return "kill [SIGKILL]";
#endif /* SIGKILL */

#ifdef SIGBUS
	case SIGBUS:
		return "bus error [SIGBUS]";
#endif /* SIGBUS */

#ifdef SIGSEGV
	case SIGSEGV:
		return "segmentation violation [SIGSEGV]";
#endif /* SIGSEGV */

#ifdef SIGSYS
	case SIGSYS:
		return "bad argument to system call [SIGSYS]";
#endif /* SIGSYS */

#ifdef SIGPIPE
	case SIGPIPE:
		return "write on a pipe with no one to read it [SIGPIPE]";
#endif /* SIGPIPE */

#ifdef SIGALRM
	case SIGALRM:
		return "alarm clock [SIGALRM]";
#endif /* SIGALRM */

#ifdef SIGTERM
	case SIGTERM:
		return "software termination [SIGTERM]";
#endif /* SIGTERM */

#ifdef SIGUSR1
	case SIGUSR1:
		return "user defined signal one [SIGUSR1]";
#endif /* SIGUSR1 */

#ifdef SIGUSR2
	case SIGUSR2:
		return "user defined signal two [SIGUSR2]";
#endif /* SIGUSR2 */

#ifdef SIGCLD
	case SIGCLD:
		return "death of child [SIGCLD]";
#endif /* SIGCLD */

#ifdef SIGPWR
	case SIGPWR:
		return "power failure [SIGPWR]";
#endif /* SIGPWR */

	default:
		sprintf(buffer, "signal %d", n);
		return buffer;
	}
}


static	quit_ty	quit_list[10];
static	int	quit_list_len;


void
quit_register(func)
	quit_ty	func;
{
	assert(quit_list_len < SIZEOF(quit_list));
	assert(func);
	quit_list[quit_list_len++] = func;
}


void
quit(n)
	int		n;
{
	int		j;
	static int	quitting;

	if (quitting > 4)
	{
		fprintf
		(
			stderr,
			"%s: incorrectly handled error while quitting (bug)\n",
			option_progname_get()
		);
		exit(1);
	}
	++quitting;
	for (j = quit_list_len - 1; j >= 0; --j)
		quit_list[j](n);
	exit(n);
}
