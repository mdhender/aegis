//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1997, 1999, 2002-2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/arglex2.h>
#include <common/error.h>	// for assert
#include <libaegis/help.h>
#include <libaegis/option.h>
#include <common/page.h>
#include <libaegis/sub.h>
#include <common/trace.h>


#define DEFAULT_TAB_WIDTH 8

#define LEVEL_TERSE 1
#define LEVEL_UNFORMATTED 2
#define LEVEL_UNSET 3
#define LEVEL_VERBOSE 4

static int      verbose_flag = LEVEL_UNSET;
static int      tab_width;
static int      page_headers = -1;



static void
too_many(void (*usage)(void))
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
option_verbose_set(void (*usage)(void))
{
    trace(("option_set_werbose()\n{\n"));
    if (verbose_flag == LEVEL_VERBOSE)
	duplicate_option_by_name(arglex_token_verbose, usage);
    if (verbose_flag != LEVEL_UNSET)
	too_many(usage);
    verbose_flag = LEVEL_VERBOSE;
    trace(("}\n"));
}


int
option_verbose_get(void)
{
    return (verbose_flag == LEVEL_VERBOSE);
}


void
option_terse_set(void (*usage)(void))
{
    trace(("option_set_terse()\n{\n"));
    if (verbose_flag == LEVEL_TERSE)
	duplicate_option_by_name(arglex_token_terse, usage);
    if (verbose_flag != LEVEL_UNSET)
	too_many(usage);
    verbose_flag = LEVEL_TERSE;
    trace(("}\n"));
}


int
option_terse_get(void)
{
    return (verbose_flag == LEVEL_TERSE);
}


void
option_unformatted_set(void (*usage)(void))
{
    trace(("option_set_unformatted()\n{\n"));
    if (verbose_flag == LEVEL_UNFORMATTED)
	duplicate_option_by_name(arglex_token_unformatted, usage);
    if (verbose_flag != LEVEL_UNSET)
	too_many(usage);
    verbose_flag = LEVEL_UNFORMATTED;
    trace(("}\n"));
}


int
option_unformatted_get(void)
{
    return (verbose_flag == LEVEL_TERSE || verbose_flag == LEVEL_UNFORMATTED);
}


void
option_page_width_set(int n, void (*usage)(void))
{
    sub_context_ty  *scp;

    switch (page_width_set(n))
    {
    case PAGE_SET_ERROR_DUPLICATE:
	duplicate_option_by_name(arglex_token_page_width, usage);
	// NOTREACHED

    case PAGE_SET_ERROR_RANGE:
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", n);
	error_intl(scp, i18n("page width $number out of range"));
	sub_context_delete(scp);
	usage();
	// NOTREACHED
    }
}


void
option_page_length_set(int n, void (*usage)(void))
{
    sub_context_ty  *scp;

    switch (page_length_set(n))
    {
    case PAGE_SET_ERROR_DUPLICATE:
    	duplicate_option_by_name(arglex_token_page_length, usage);
	// NOTREACHED

    case PAGE_SET_ERROR_RANGE:
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", n);
	error_intl(scp, i18n("page length $number out of range"));
	sub_context_delete(scp);
	usage();
	// NOTREACHED
    }
}


void
option_tab_width_set(int n, void (*usage)(void))
{
    if (tab_width)
	duplicate_option_by_name(arglex_token_tab_width, usage);
    if (n < 0 || n > 40)
    {
	sub_context_ty *scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", n);
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
option_tab_width_get(void)
{
    if (!tab_width)
	tab_width = DEFAULT_TAB_WIDTH;
    return (tab_width < 2 ? 0 : tab_width);
}


void
option_page_headers_set(int n, void (*usage)(void))
{
    switch (page_headers)
    {
    case 0:
	if (n)
	{
	    mutually_exclusive_options
	    (
		arglex_token_page_headers,
		arglex_token_page_headers_not,
		usage
	    );
	}
	else
	{
	    duplicate_option_by_name(arglex_token_page_headers_not, usage);
	}
	break;

    case 1:
	if (n)
	{
	    duplicate_option_by_name(arglex_token_page_headers, usage);
	}
	else
	{
	    mutually_exclusive_options
	    (
		arglex_token_page_headers,
		arglex_token_page_headers_not,
		usage
	    );
	}
	break;

    default:
	page_headers = !!n;
	break;
    }
}


int
option_page_headers_get(void)
{
    if (page_headers < 0)
	page_headers = 1;
    return page_headers;
}


static int signed_off_by = -1;


void
option_signed_off_by_argument(void (*usage)(void))
{
    switch (arglex_token)
    {
    case arglex_token_signed_off_by_not:
	if (signed_off_by > 0)
	{
	    mutually_exclusive_options
	    (
		arglex_token_signed_off_by,
		arglex_token_signed_off_by_not,
		usage
	    );
	}
	else if (signed_off_by == 0)
	{
	    duplicate_option_by_name(arglex_token_signed_off_by_not, usage);
	}
        signed_off_by = 0;
        break;

    case arglex_token_signed_off_by:
	if (signed_off_by > 0)
	{
	    duplicate_option_by_name(arglex_token_signed_off_by, usage);
	}
	else if (signed_off_by == 0)
	{
	    mutually_exclusive_options
	    (
		arglex_token_signed_off_by,
		arglex_token_signed_off_by_not,
		usage
	    );
	}
        signed_off_by = 1;
	break;
    }
}


bool
option_signed_off_by_get(bool dflt)
{
    return (signed_off_by < 0 ? dflt : (signed_off_by != 0));
}
