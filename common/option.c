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
 * MANIFEST: functions to manipulate command line options
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <error.h>
#include <option.h>
#include <trace.h>


#define MIN_PAGE_WIDTH 40
/* #define MAX_PAGE_WIDTH in option.h */
#define DEFAULT_PAGE_WIDTH 80
#define MIN_PAGE_LENGTH 10
#define MAX_PAGE_LENGTH 30000
#define DEFAULT_PAGE_LENGTH 24

#define LEVEL_TERSE 1
#define LEVEL_UNFORMATTED 2
#define LEVEL_UNSET 3
#define LEVEL_VERBOSE 4

static	int	page_length;
static	char	*progname;
static	int	verbose_flag = LEVEL_UNSET;
static	int	page_width;


void
option_progname_set(s)
	char		*s;
{
	/* do NOT put tracing in this function */
	assert(s);
	assert(!progname);
	for (;;)
	{
		progname = strrchr(s, '/');

		/*
		 * we were invoked as
		 *	progname -args
		 */
		if (!progname)
		{
			progname = s;
			break;
		}

		/*
		 * we were invoked as
		 *	/usr/local/progname -args
		 */
		if (progname[1])
		{
			++progname;
			break;
		}

		/*
		 * this is real nasty:
		 * it is possible to invoke us as
		 *	/usr//local///bin////progname///// -args
		 * and it is legal!!
		 */
		*progname = 0;
	}
}


char *
option_progname_get()
{
	/* do NOT put tracing in this function */
	assert(progname);
	return progname;
}


static void too_many _((void));

static void
too_many()
{
	fatal
	(
	     "only one of -TERse and -UNFormatted and -Verbose may be specified"
	);
}


void
option_verbose_set()
{
	trace(("option_set_verbose()\n{\n"/*}*/));
	if (verbose_flag == LEVEL_VERBOSE)
		fatal("duplicate -Verbose option");
	if (verbose_flag != LEVEL_UNSET)
		too_many();
	verbose_flag = LEVEL_VERBOSE;
	trace((/*{*/"}\n"));
}


int
option_verbose_get()
{
	return (verbose_flag == LEVEL_VERBOSE);
}


void
option_terse_set()
{
	trace(("option_set_terse()\n{\n"/*}*/));
	if (verbose_flag == LEVEL_TERSE)
		fatal("duplicate -TERse option");
	if (verbose_flag != LEVEL_UNSET)
		too_many();
	verbose_flag = LEVEL_TERSE;
	trace((/*{*/"}\n"));
}


int
option_terse_get()
{
	return (verbose_flag == LEVEL_TERSE);
}


void
option_unformatted_set()
{
	trace(("option_set_unformatted()\n{\n"/*}*/));
	if (verbose_flag == LEVEL_UNFORMATTED)
		fatal("duplicate -UNFormatted option");
	if (verbose_flag != LEVEL_UNSET)
		too_many();
	verbose_flag = LEVEL_UNFORMATTED;
	trace((/*{*/"}\n"));
}


int
option_unformatted_get()
{
	return
	(
		verbose_flag == LEVEL_TERSE
	||
		verbose_flag == LEVEL_UNFORMATTED
	);
}


static void default_page_sizes _((void));

static void
default_page_sizes()
{
	if (!page_width)
	{
		char	*cp;

		cp = getenv("COLS");
		if (cp)
		{
			int	n;

			n = atoi(cp);
			if (n > 0)
			{
				if (n < MIN_PAGE_WIDTH)
					n = MIN_PAGE_WIDTH;
				if (n > MAX_PAGE_WIDTH)
					n = MAX_PAGE_WIDTH;
				page_width = n;
			}
		}
	}
	if (!page_length)
	{
		char	*cp;

		cp = getenv("LINES");
		if (cp)
		{
			int	n;

			n = atoi(cp);
			if (n > 0)
			{
				if (n < MIN_PAGE_LENGTH)
					n = MIN_PAGE_LENGTH;
				if (n > MAX_PAGE_LENGTH)
					n = MAX_PAGE_LENGTH;
				page_length = n;
			}
		}
	}

#ifdef TIOCGWINSZ
	if (!page_width || !page_length)
	{
		struct winsize	ws;

		if (ioctl(0, TIOCGWINSZ, &ws) == 0)
		{
			if (!page_width && ws.ws_col > 0)
			{
				page_width = ws.ws_col;
				if (page_width < MIN_PAGE_WIDTH)
					page_width = MIN_PAGE_WIDTH;
				if (page_width > MAX_PAGE_WIDTH)
					page_width = MAX_PAGE_WIDTH;
			}
			if (!page_length && ws.ws_row > 0)
			{
				page_length = ws.ws_row;
				if (page_length < MIN_PAGE_LENGTH)
					page_length = MIN_PAGE_LENGTH;
				if (page_length > MAX_PAGE_LENGTH)
					page_length = MAX_PAGE_LENGTH;
			}
		}
	}
#endif

	if (!page_width)
		page_width = DEFAULT_PAGE_WIDTH;
	if (!page_length)
		page_length = DEFAULT_PAGE_LENGTH;
}


void
option_page_width_set(n)
	int	n;
{
	if (page_width)
		fatal("duplicate -Page_Width option");
	if (n < MIN_PAGE_WIDTH || n > MAX_PAGE_WIDTH)
		fatal("page width %d out of range", n);
	page_width = n;
}


int
option_page_width_get()
{
	/*
	 * must not generate a fatal error in this function,
	 * as it is used by 'error.c' when reporting fatal errors.
	 *
	 * must not put tracing in this function,
	 * because 'trace.c' uses it to determine the width.
	 */
	if (!page_width)
		default_page_sizes();
	return page_width;
}


void
option_page_length_set(n)
	int	n;
{
	if (page_length)
		fatal("duplicate -Page_Length option");
	if (n < MIN_PAGE_LENGTH || n > MAX_PAGE_LENGTH)
		fatal("page length %d out of range", n);
	page_length = n;
}


int
option_page_length_get()
{
	if (!page_length)
		default_page_sizes();
	return page_length;
}
