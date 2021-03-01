/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate command line options
 */

#include <ac/stdlib.h>
#include <ac/termios.h>

#include <arglex2.h>
#include <help.h>
#include <option.h>
#include <sub.h>
#include <trace.h>


#define MIN_PAGE_WIDTH 40
/* #define MAX_PAGE_WIDTH in option.h */
#define DEFAULT_PAGE_WIDTH 80
#define MIN_PAGE_LENGTH 10
#define MAX_PAGE_LENGTH 30000
#define DEFAULT_PAGE_LENGTH 24

#define DEFAULT_TAB_WIDTH 8

#define LEVEL_TERSE 1
#define LEVEL_UNFORMATTED 2
#define LEVEL_UNSET 3
#define LEVEL_VERBOSE 4

static	int	page_length;
static	int	verbose_flag = LEVEL_UNSET;
static	int	page_width;
static	int	tab_width;



static void too_many _((void (*usage)(void)));

static void
too_many(usage)
	void		(*usage)_((void));
{
	mutually_exclusive_options3
	(
		arglex_token_terse,
		arglex_token_unformatted,
		arglex_token_verbose,
		usage
	);
}


void
option_verbose_set(usage)
	void		(*usage)_((void));
{
	trace(("option_set_werbose()\n{\n"/*}*/));
	if (verbose_flag == LEVEL_VERBOSE)
		duplicate_option_by_name(arglex_token_verbose, usage);
	if (verbose_flag != LEVEL_UNSET)
		too_many(usage);
	verbose_flag = LEVEL_VERBOSE;
	trace((/*{*/"}\n"));
}


int
option_verbose_get()
{
	return (verbose_flag == LEVEL_VERBOSE);
}


void
option_terse_set(usage)
	void		(*usage)_((void));
{
	trace(("option_set_terse()\n{\n"/*}*/));
	if (verbose_flag == LEVEL_TERSE)
		duplicate_option_by_name(arglex_token_terse, usage);
	if (verbose_flag != LEVEL_UNSET)
		too_many(usage);
	verbose_flag = LEVEL_TERSE;
	trace((/*{*/"}\n"));
}


int
option_terse_get()
{
	return (verbose_flag == LEVEL_TERSE);
}


void
option_unformatted_set(usage)
	void		(*usage)_((void));
{
	trace(("option_set_unformatted()\n{\n"/*}*/));
	if (verbose_flag == LEVEL_UNFORMATTED)
		duplicate_option_by_name(arglex_token_unformatted, usage);
	if (verbose_flag != LEVEL_UNSET)
		too_many(usage);
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
option_page_width_set(n, usage)
	int		n;
	void		(*usage)_((void));
{
	if (page_width)
		duplicate_option_by_name(arglex_token_page_width, usage);
	if (n < MIN_PAGE_WIDTH || n > MAX_PAGE_WIDTH)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", n);
		error_intl(scp, i18n("page width $number out of range"));
		sub_context_delete(scp);
		usage();
	}
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
option_page_length_set(n, usage)
	int		n;
	void		(*usage)_((void));
{
	if (page_length)
		duplicate_option_by_name(arglex_token_page_length, usage);
	if (n < MIN_PAGE_LENGTH || n > MAX_PAGE_LENGTH)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", n);
		error_intl(scp, i18n("page length $number out of range"));
		sub_context_delete(scp);
		usage();
	}
	page_length = n;
}


int
option_page_length_get()
{
	if (!page_length)
		default_page_sizes();
	return page_length;
}


void
option_tab_width_set(n, usage)
	int		n;
	void		(*usage)_((void));
{
	if (tab_width)
		duplicate_option_by_name(arglex_token_tab_width, usage);
	if (n < 0 || n > 40)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", n);
		error_intl(scp, i18n("tab width $number out of range"));
		sub_context_delete(scp);
		usage();
	}
	if (n < 2)
		tab_width = -1;
	else
		tab_width = n;
}


int
option_tab_width_get()
{
	if (!tab_width)
		tab_width = DEFAULT_TAB_WIDTH;
	return (tab_width < 2 ? 0 : tab_width);
}
