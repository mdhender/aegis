//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1997, 1999, 2002-2006, 2008, 2009, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>

#include <fmtgen/arglex.h>
#include <fmtgen/generator/orig_code.h>
#include <fmtgen/generator/orig_include.h>
#include <fmtgen/generator/pair.h>
#include <fmtgen/lex.h>
#include <fmtgen/parse.h>


static void
usage(void)
{
    const char *progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s [ <option>... ] <file.def> <file.c> <libaegis/file.h>\n",
        progname
    );
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
help(void)
{
    static const char *const text[] =
    {
"NAME",
"       %s - file format read/write generator",
"",
"SYNOPSIS",
"       %s [ <option>... ] file.def file.c file.h",
"       %s -Help",
"",
"DESCRIPTION",
"       The %s program is used to process the .def files",
"       describing a file format into the .c and .h files which",
"       implement that format (with the help of gram.y and",
"       indent.c).",
"",
"OPTIONS",
"       The following options are understood:",
"",
"       -Help",
"               Give this help message.",
"",
"       -I<path>",
"               Specify and include search path.",
"",
"       All options may be abbreviated; the abbreviation is",
"       documented as the upper case letters, all lower case",
"       letters and underscores (_) are optional.  You must use",
"       consecutive sequences of optional letters.",
"",
"       All options are case insensitive, you may type them in",
"       upper case or lower case or a combination of both, case",
"       is not important.",
"",
"       For example: the arguments \"-project, \"-PROJ\" and \"-p\"",
"       are all interpreted to mean the -Project option.  The",
"       argument \"-prj\" will not be understood, because",
"       consecutive optional characters were not supplied.",
"",
"       Options and other command line arguments may be mixed",
"       arbitrarily on the command line, after the function",
"       selectors.",
"",
"       The GNU long option names are understood.  Since all",
"       option names for aegis are long, this means ignoring the",
"       extra leading '-'.  The \"--option=value\" convention is",
"       also understood.",
"",
"EXIT STATUS",
"       The %s program will exit with a status of 1 on any",
"       error.  The %s program will only exit with a status",
"       of 0 if there are no errors.",
"",
"COPYRIGHT",
"       The %s program is Copyright (C) 1990, 1991, 1992, 1993,",
"",
"       The %s program comes with ABSOLUTELY NO WARRANTY; for",
"       details use the 'aegis -VERSion Warranty' command.  This",
"       is free software and you are welcome to redistribute it",
"       under certain conditions; for details use the 'aegis",
"       -VERSion Redistribution' command.",
"",
"AUTHOR",
"       Peter Miller   E-Mail: pmiller@opensource.org.au",
"       /\\/\\*             WWW: http://miller.emu.id.au/pmiller/",
            0
    };

    trace(("help()\n{\n"));
    const char *progname = progname_get();
    for (const char *const *cpp = text; *cpp; ++cpp)
    {
        const char *line = *cpp;
        for (;;)
        {
            const char *p = strstr(line, "%s");
            if (!p)
                break;
            fwrite(line, p - line, 1, stdout);
            line = p + 2;
            fputs(progname, stdout);
        }
        fputs(line, stdout);
        putchar('\n');
    }
    trace(("}\n"));
}


int
main(int argc, char **argv)
{
    arglex2_init(argc, argv);
    if (arglex() == arglex_token_help)
    {
        if (arglex() != arglex_token_eoln)
            usage();
        help();
        quit(0);
    }

    nstring_list filename;
    generator::pointer gen;
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

        case arglex_token_include_long:
            if (arglex() != arglex_token_string)
                usage();
            // fall through...

        case arglex_token_include_short:
            lex_include_path(arglex_value.alv_string);
            break;

        case arglex_token_string:
            filename.push_back(arglex_value.alv_string);
            break;

        case arglex_token_introspector_code:
        case arglex_token_introspector_include:
        case arglex_token_original_code:
        case arglex_token_original_include:
            {
                int tok = arglex_token;
                if (arglex() != arglex_token_string)
                {
                    fatal_raw
                    (
                        "%s option requires filename",
                        arglex_token_name(tok)
                    );
                }
                nstring outfilename = arglex_value.alv_string;
                generator::pointer gp = generator::factory(tok, outfilename);
                gen = (gen ? generator_pair::create(gen, gp) : gp);
            }
            break;

        case arglex_token_tab_width:
            if (arglex() != arglex_token_number)
            {
                fatal_raw
                (
                    "%s option requires number",
                    arglex_token_name(arglex_token_tab_width)
                );
            }
            indent::set_indent(arglex_value.alv_number);
            break;

        case arglex_token_bit_fields:
        case arglex_token_bit_fields_not:
            {
                bool yesno = (arglex_token == arglex_token_bit_fields);
                type::set_bit_fields(yesno);
            }
            break;

#ifdef DEBUG
        case arglex_token_trace:
            while (arglex() == arglex_token_string)
                trace_enable(arglex_value.alv_string);
            continue;
#endif
        }
        arglex();
    }
    if (gen)
    {
        if (filename.size() < 1)
            fatal_raw("too few file names");
        else if (filename.size() > 1)
        {
            fatal_raw
            (
                "too many file names (got %d, expected 1)",
                int(filename.size())
            );
        }
    }
    else
    {
        if (filename.size() < 3)
        {
            fatal_raw
            (
                "too few file names (got %d, expected 3)",
                int(filename.size())
            );
        }
        else if (filename.size() > 3)
        {
            fatal_raw
            (
                "too many file names (got %d, expected 3)",
                int(filename.size())
            );
        }
        generator::pointer g1 = generator_original_code::create(filename[1]);
        generator::pointer g2 = generator_original_include::create(filename[2]);
        gen = generator_pair::create(g1, g2);
    }

    parse(gen, filename[0]);
    quit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
