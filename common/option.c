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
#define DEFAULT_PAGE_WIDTH 79 /* don't use 80, many terminals really stupid */
#define MIN_PAGE_LENGTH 10
#define MAX_PAGE_LENGTH 30000
#define DEFAULT_PAGE_LENGTH 23 /* assume 24 line terminal, leave 1 for pager */

static	int	page_length;
static	char	*progname;
static	int	verbose_flag;
static	int	page_width;


void
option_set_progname(s)
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
option_get_progname()
{
	/* do NOT put tracing in this function */
	assert(progname);
	return progname;
}


void
option_set_verbose()
{
	trace(("option_set_verbose()\n{\n"/*}*/));
	if (verbose_flag < 0)
		fatal("only one of -TERse and -Verbose may be specified");
	if (verbose_flag)
		fatal("duplicate -Verbose option");
	verbose_flag = 1;
	trace((/*{*/"}\n"));
}


int
option_get_verbose()
{
	return (verbose_flag > 0);
}


void
option_set_terse()
{
	trace(("option_set_terse()\n{\n"/*}*/));
	if (verbose_flag > 0)
		fatal("only one of -TERse and -Verbose may be specified");
	if (verbose_flag)
		fatal("duplicate -TERse option");
	verbose_flag = -1;
	trace((/*{*/"}\n"));
}


int
option_get_terse()
{
	return (verbose_flag < 0);
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
			/*
			 * Don't use right-most column, some terminals are dumb.
			 */
			page_width = atoi(cp) - 1;
			if (page_width < MIN_PAGE_WIDTH)
				page_width = MIN_PAGE_WIDTH;
			if (page_width > MAX_PAGE_WIDTH)
				page_width = MAX_PAGE_WIDTH;
		}
	}
	if (!page_length)
	{
		char	*cp;

		cp = getenv("LINES");
		if (cp)
		{
			/*
			 * Don't use bottom line, leave it for the pager.
			 */
			page_length = atoi(cp) - 1;
			if (page_length < MIN_PAGE_LENGTH)
				page_length = MIN_PAGE_LENGTH;
			if (page_length > MAX_PAGE_LENGTH)
				page_length = MAX_PAGE_LENGTH;
		}
	}

#ifdef TIOCGWINSZ
	if (!page_width || !page_length)
	{
		struct winsize	ws;

		if (ioctl(0, TIOCGWINSZ, &ws) == 0)
		{
			/*
			 * Don't use right-most column, some terminals are dumb.
			 * Don't use bottom line, leave it for the pager.
			 */
			if (!page_width)
			{
				page_width = ws.ws_col - 1;
				if (page_width < MIN_PAGE_WIDTH)
					page_width = MIN_PAGE_WIDTH;
				if (page_width > MAX_PAGE_WIDTH)
					page_width = MAX_PAGE_WIDTH;
			}
			if (!page_length)
			{
				page_length = ws.ws_row - 1;
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
option_set_page_width(n)
	int	n;
{
	if (page_width)
		fatal("duplicate -Page_Width option");
	if (n < MIN_PAGE_WIDTH || n > MAX_PAGE_WIDTH)
		fatal("page width %d out of range", n);
	page_width = n;
}


int
option_get_page_width()
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
option_set_page_length(n)
	int	n;
{
	if (page_length)
		fatal("duplicate -Page_Length option");
	if (n < MIN_PAGE_LENGTH || n > MAX_PAGE_LENGTH)
		fatal("page length %d out of range", n);
	page_length = n;
}


int
option_get_page_length()
{
	if (!page_length)
		default_page_sizes();
	return page_length;
}
