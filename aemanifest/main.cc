//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/env.h>
#include <common/error.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/str_list.h>
#include <libaegis/arglex2.h>
#include <libaegis/dir_stack.h>
#include <libaegis/help.h>
#include <libaegis/os.h>


static void
usage(void)
{
    const char      *prog;

    prog = progname_get();
    fprintf(stderr, "usage: %s [ -dir=<path> ][ -o <file> ] <file>...\n", prog);
    quit(1);
}


struct table_ty
{
    const char      *name;
    const char      *description;
};

static const table_ty table[] =
{
    { "MANIFEST",	"This file"					},
    { "LICENSE",	"GNU General Public License"			},
    { "README",		"Blurb about the project"			},
    { "BUILDING",	"Instructions how to build, test and install"	},
    { "etc/CHANGES.*",	"Change history of the project"			},
    { "Makefile*",	"Instructions to make(1) how to build and test"	},
    { "common/patchlevel.h", "The patch level of this distribution."	},
    { "doc/version.so",	"The patch level of this distribution."		},
    { "etc/new.so",	"Include redirection for release notes."	},
    { "etc/config.h*",	"Template information for common/config.h.in"	},
    { "etc/template/*",	"New file template"				},
    { "lib/aegis.icon",	"X Bitmap of the Aegis icon"			},
    { "lib/aegis.mask",	"X Bitmap of the mask for the Aegis icon"	},
    { "configure",	"Shell script to automagically configure"	},
    { "common/config.h.in", "Template for configuration definitions."	},
};


static const char *
find(string_ty *fn)
{
    const table_ty  *tp;

    for (tp = table; tp < ENDOF(table); ++tp)
    {
	const char      *cp;

	cp = strchr(tp->name, '*');
	if (cp)
	{
	    size_t          nbytes;

	    nbytes = cp - tp->name;
	    if
	    (
		fn->str_length >= nbytes
	    &&
		0 == memcmp(fn->str_text, tp->name, nbytes)
	    )
		return tp->description;
	}
	else
	{
	    if (0 == strcmp(fn->str_text, tp->name))
		return tp->description;
	}
    }
    return 0;
}


int
main(int argc, char **argv)
{
    size_t	    j;
    string_ty       *s;
    const char      *ofn;
    FILE            *ofp;

    os_become_init_mortal();
    resource_limits_init();
    arglex2_init(argc, argv);
    env_initialize();
    language_init();

    arglex();
    string_list_ty search_path;
    string_list_ty filename;
    ofn = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, usage);
	    s = str_from_c(arglex_value.alv_string);
	    search_path.push_back(s);
	    str_free(s);
	    break;

	case arglex_token_string:
	    s = str_from_c(arglex_value.alv_string);
	    filename.push_back(s);
    	    str_free(s);
	    break;

	case arglex_token_output:
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_directory, usage);
	    ofn = arglex_value.alv_string;
	    break;
	}
	arglex();
    }
    if (!search_path.nstrings)
	search_path.push_back(str_from_c("."));
    if (ofn)
    {
	ofp = fopen(ofn, "w");
	if (!ofp)
	    nfatal("open %s", ofn);
    }
    else
	ofp = stdout;

    fprintf(ofp, "\n\nMANIFEST\tThis file.\n");
    for (j = 0; j < filename.nstrings; ++j)
    {
	string_ty       *fn;
	FILE            *fp;

	fn = filename.string[j];
	s = dir_stack_find(&search_path, 0, fn, 0, 0, 0);
	if (!s)
	    fatal_raw("%s: no such file", fn->str_text);
	fp = fopen(s->str_text, "r");
	if (!fp)
	    nfatal("open %s", s->str_text);
	fprintf(ofp, "%s\t", fn->str_text);
	for (;;)
	{
	    char            buffer[2000];
	    const char      *cp;

	    if (!fgets(buffer, sizeof(buffer), fp))
	    {
		cp = find(fn);
		fprintf(ofp, "%s\n", cp ? cp : "no manifest specified");
		break;
	    }
            // make sure this line does not match the
            // aede-policy filter
	    cp = strstr(buffer, "MANIFEST" ":");
	    if
	    (
		cp
	    &&
		(cp == buffer || isspace((unsigned char)cp[-1]))
	    &&
		isspace((unsigned char)cp[9])
	    )
	    {
		fputs(cp + 10, ofp);
		break;
	    }
	}
	fclose(fp);
    }
    if (ofn && fclose(ofp))
	nfatal("write %s", ofn);

    //
    // report success
    //
    quit(0);
    return 0;
}
