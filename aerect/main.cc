//
//	aegis - project change supervisor
//	Copyright (C) 1998-2000, 2002-2006, 2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <libaegis/arglex2.h>
#include <common/env.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <common/quit.h>
#include <aeget/rect.h>
#include <libaegis/sub.h>
#include <libaegis/version.h>


enum
{
    arglex_token_bevel = ARGLEX2_MAX,
    arglex_token_color,
    arglex_token_label,
    arglex_token_mime,
    arglex_token_mime_not,
    ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
    { "_Bevel",   arglex_token_bevel,    },
    { "_Color",   arglex_token_color,    },
    { "_Colour",  arglex_token_color,    },
    { "_Label",   arglex_token_label,    },
    { "-Mime",    arglex_token_mime,     },
    { "-No_Mime", arglex_token_mime_not, },
    ARGLEX_END_MARKER
};


static void
rect_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] width height\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
rect_help(void)
{
    help((char *)0, rect_usage);
}


static int
get_bevel_size(void)
{
    int             n;

    if (arglex() != arglex_token_number)
    {
	option_needs_number
	(
    	    arglex_token_bevel,
    	    rect_usage
	);
	rect_usage();
    }
    n = arglex_value.alv_number;

    if (n < 0 || n > 10)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", n);
	error_intl(scp, i18n("bevel size $number out of range"));
	rect_usage();
    }

    return n;
}


static int
get_color_component(void)
{
    int             n;

    if (arglex() != arglex_token_number)
    {
	option_needs_number(arglex_token_color, rect_usage);
	rect_usage();
    }
    n = arglex_value.alv_number;

    if (n < 0 || n >= 256)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", n);
	error_intl(scp, i18n("color component $number out of range"));
	rect_usage();
    }

    return n;
}


int
main(int argc, char **argv)
{
    int             width;
    int             height;
    const char      *outfile;
    int		    r, g, b;
    const char      *label;

    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
	rect_help();
	quit(0);

    case arglex_token_version:
	version();
	quit(0);
    }

    width = 0;
    height = 0;
    outfile = 0;
    label = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(rect_usage);
	    continue;

	case arglex_token_number:
	    if (arglex_value.alv_number < 1)
		arglex_value.alv_number = 1;
	    if (!width)
		width = arglex_value.alv_number;
	    else if (!height)
		height = arglex_value.alv_number;
	    else
	    {
		error_intl(0, i18n("too many sizes specified"));
		rect_usage();
	    }
	    break;

	case arglex_token_output:
	    if (outfile)
		duplicate_option(rect_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file
		(
	    	    arglex_token_output,
	    	    rect_usage
		);
		rect_usage();

	    case arglex_token_stdio:
		outfile = "";
		break;

	    case arglex_token_string:
		outfile = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_bevel:
	    rect_bevel(get_bevel_size());
	    break;

	case arglex_token_color:
	    r = get_color_component();
	    g = get_color_component();
	    b = get_color_component();
	    rect_color(r, g, b);
	    break;

	case arglex_token_label:
	    switch (arglex())
	    {
	    default:
		option_needs_string
		(
	    	    arglex_token_output,
	    	    rect_usage
		);
		rect_usage();
		// NOTREACHED

	    case arglex_token_string:
	    case arglex_token_number:
		label = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_mime:
	    rect_mime(1);
	    break;

	case arglex_token_mime_not:
	    rect_mime(0);
	    break;
	}
	arglex();
    }
    if (outfile && !outfile)
	outfile = 0;
    if (width && !height)
	height = width;
    if (!width || !height)
	fatal_intl(0, i18n("no size specified"));
    rect(outfile, width, height, label);
    quit(0);
    return 0;
}
