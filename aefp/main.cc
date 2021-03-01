//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1995, 1997, 1999, 2002-2006, 2008 Peter Miller
//	Copyright (C) 2007 Walter Franzini
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
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/error.h>
#include <common/fp/cksum.h>
#include <common/fp/cksum.h>
#include <common/fp/combined.h>
#include <common/fp/ident.h>
#include <common/fp/md5.h>
#include <common/fp/snefru.h>
#include <common/progname.h>
#include <common/str_list.h>

#include <libaegis/help.h>
#include <libaegis/os.h>


enum
{
    arglex_token_cksum,
    arglex_token_ident,
    arglex_token_md5,
    arglex_token_snefru
};

static arglex_table_ty argtab[] =
{
    { "-Checksum",       arglex_token_cksum,	},
    { "-Identifier",     arglex_token_ident,	},
    { "-Message_Digest", arglex_token_md5,	},
    { "-Snefru",         arglex_token_snefru,	},
    { 0, 0, } // end marker
};


static void
usage(void)
{
    fprintf
    (
	stderr,
	"Usage: %s [ <option>... ][ <filename>... ]\n",
	progname_get()
    );
    exit(1);
}


int
main(int argc, char **argv)
{
    string_ty	    *minus;
    size_t	    j;
    fingerprint_methods_ty *method;
    string_ty	    *s;

    os_become_init_mortal();

    arglex_init(argc, argv, argtab);
    arglex();

    method = 0;
    string_list_ty file;
    minus = str_from_c("-");
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    error_raw
	    (
		"misplaced \"%s\" command line argument",
		arglex_value.alv_string
	    );
	    usage();

	case arglex_token_help:
	    help(progname_get(), usage);
            exit(0);
            // NOTREACHED

	case arglex_token_snefru:
	    if (method)
	    {
		too_many_methods:
		error_raw("too many methods specified");
		usage();
	    }
	    method = &fp_snefru;
	    break;

	case arglex_token_ident:
	    if (method)
		goto too_many_methods;
	    method = &fp_ident;
	    break;

	case arglex_token_md5:
	    if (method)
		goto too_many_methods;
	    method = &fp_md5;
	    break;

	case arglex_token_cksum:
	    if (method)
		goto too_many_methods;
	    method = &fp_cksum;
	    break;

	case arglex_token_string:
	    s = str_from_c(arglex_value.alv_string);
	    file.push_back(s);
	    str_free(s);
	    break;

	case arglex_token_stdio:
	    if (file.member(minus))
	    {
		error_raw("may only name stdin once");
		usage();
	    }
	    file.push_back(minus);
	    break;
	}
	arglex();
    }

    //
    // if no files named, read stdin
    //
    if (!file.nstrings)
	file.push_back(minus);

    //
    // by default, use the fp_combined class
    //
    if (!method)
	method = &fp_combined;

    //
    // read the named files
    //
    for (j = 0; j < file.nstrings; ++j)
    {
	fingerprint_ty	*p;
	char		buf[1000];

	p = fingerprint_new(method);
	s = file.string[j];
	if (str_equal(s, minus))
	{
	    if (fingerprint_file_sum(p, (char *)0, buf, sizeof(buf)))
	       	nfatal("standard input");
	    printf("%s", buf);
	    if (file.nstrings != 1)
	       	printf("\tstdin");
	    printf("\n");
	}
	else
	{
	    if (fingerprint_file_sum(p, s->str_text, buf, sizeof(buf)))
	       	nfatal("%s", s->str_text);
	    printf("%s\t%s\n", buf, s->str_text);
	}
	fingerprint_delete(p);
    }
    exit(0);
    return 0;
}
