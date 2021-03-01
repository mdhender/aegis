/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1998, 1999 Peter Miller;
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
 * MANIFEST: functions to report errors
 */

#include <ac/ctype.h>
#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/libintl.h>

#include <ac/unistd.h>
#include <ac/stdarg.h>

#include <arglex.h>
#include <error.h>
#include <mprintf.h>
#include <progname.h>


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

static void wrap _((const char *));

static void
wrap(s)
	const char	*s;
{
	char		*progname;
	static char	escapes[] = "\rr\nn\ff\bb\tt";
	int		page_width;
	char		tmp[200];
	int		first_line;
	char		*tp;

	if (fflush(stdout) || ferror(stdout))
		nfatal("(stdout)");
	/* don't use last column, many terminals are dumb */
	page_width = 79;
	progname = progname_get();
	first_line = 1;
	while (*s)
	{
		const char	*ep;
		int		ocol;

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
			const char	*mp;
			const char	*bp_space;
			const char	*bp_slash;

			bp_space = 0;
			for (mp = ep; mp > s; --mp)
			{
				if (mp[-1] == ' ')
				{
					bp_space = mp;
					break;
				}
			}

			bp_slash = 0;
			for (mp = ep; mp > s; --mp)
			{
				if (strchr("\\/", mp[-1]))
				{
					bp_slash = mp;
					break;
				}
			}

			/*
			 * We could break it at the space, and only use
			 * the slash if there are no spaces on the line.
			 * This can lead to large amounts of wasted
			 * space, particularly for link commands.  So, if
			 * both breaks are possible, and the space break
			 * is before the slash break, and the space
			 * break is in the left half of the line, use
			 * the slash break.
			 */
			if
			(
				bp_space
			&&
				bp_slash
			&&
				bp_space < bp_slash
			&&
				bp_space < s + 30
			)
				bp_space = 0;

			/*
			 * use the break if available
			 */
			if (bp_space)	
				ep = bp_space;
			else if (bp_slash)
				ep = bp_slash;
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
				const char	*esc;

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
	{
		static int disaster_count;

		/*
		 * If there is a problem with stderr, it is usually
		 * because the tee command went away.  Try to cope, so
		 * that we can finish cleaning up, but don't try too hard.
		 */
		if (disaster_count++ || !freopen("/dev/null", "w", stderr))
			exit(1);
		quit(1);
	}
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


static char *copy_string _((const char *));

static char *
copy_string(s)
	const char	*s;
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

void
error_raw(fmt sva_last)
	const char	*fmt;
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

void
nerror(fmt sva_last)
	const char	*fmt;
	sva_last_decl
{
	char		*s1;
	va_list		ap;
	int		n;

	n = errno;
	sva_init(ap, fmt);
	s1 = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!s1)
		double_jeopardy();
	s1 = copy_string(s1);
	error_raw("%s: %s", s1, strerror(n));
	free(s1);
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
	const char	*fmt;
	sva_last_decl
{
	char		*s1;
	va_list		ap;
	int		n;

	n = errno;
	sva_init(ap, fmt);
	s1 = vmprintf_errok(fmt, ap);
	va_end(ap);
	if (!s1)
		double_jeopardy();
	s1 = copy_string(s1);

	fatal_raw("%s: %s", s1, strerror(n));
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

void
fatal_raw(fmt sva_last)
	const char	*fmt;
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
	const char	*s;
	const char	*file;
	int		line;
{
	error_raw("%s: %d: assertion \"%s\" failed (bug)", file, line, s);
	abort();
	exit(1); /* incase abort() comes back */
}


static	quit_ty	quit_list[10];
static	int	quit_list_len;
static	int	quitting;


void
quit_register(func)
	quit_ty	func;
{
	if (quitting)
		return;
	assert(quit_list_len < SIZEOF(quit_list));
	assert(func);
	quit_list[quit_list_len++] = func;
}


void
quit(n)
	int		n;
{
	if (quitting > 4)
	{
		fprintf
		(
			stderr,
			"%s: incorrectly handled error while quitting (bug)\n",
			progname_get()
		);
		exit(1);
	}
	++quitting;
	while (quit_list_len > 0)
		quit_list[--quit_list_len](n);
	exit(n);
}
