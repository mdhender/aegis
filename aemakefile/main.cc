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
// This was originally a shell script, but as the number of files grew,
// its execution time became unreasonably slow.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/env.h>
#include <common/error.h>
#include <common/language.h>
#include <common/nstring/list.h>
#include <common/rsrc_limits.h>
#include <aemakefile/printer/stdout.h>
#include <aemakefile/printer/wrap.h>
#include <aemakefile/process/body.h>
#include <aemakefile/process/tail.h>
#include <common/progname.h>


enum
{
    arglex_token_body,
    arglex_token_directory,
    arglex_token_files_from
};

static arglex_table_ty argtab[] =
{
    { "-Body", arglex_token_body },
    { "-DIRectory", arglex_token_directory },
    { "-Files_From", arglex_token_files_from },
    ARGLEX_END_MARKER
};


static void
usage()
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s <filename>...\n", progname);
    fprintf(stderr, "       %s -body <filename>\n", progname);
    exit(1);
}


static void
slurp(const char *filename, nstring_list &nsl)
{
    FILE *fp = filename ? fopen(filename, "r") : stdin;
    if (!fp)
	nfatal("open %s", filename);
    for (;;)
    {
	char buffer[2000];
	char *bp = buffer;
	int c;
	for (;;)
	{
	    c = getc(fp);
	    if (c == EOF || c == '\n')
		break;
	    if (bp < buffer + sizeof(buffer))
		*bp++ = c;
	}
	if (bp == buffer && c == EOF)
	    break;
	nsl.push_back(nstring(buffer, bp - buffer));
    }
    if (fp != stdin)
	fclose(fp);
}


int
main(int argc, char **argv)
{
    arglex_init(argc, argv, argtab);
    env_initialize();
    language_init();
    resource_limits_init();

    if (arglex() == arglex_token_help)
	usage();
    printer_wrap print(new printer_stdout);
    nstring_list filenames;
    process *proc = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    usage();

	case arglex_token_string:
	    filenames.push_back(arglex_value.alv_string);
	    break;

	case arglex_token_body:
	    if (proc)
		usage();
	    proc = new process_body(print);
	    break;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		usage();
	    process_body::directory(arglex_value.alv_string);
	    break;

	case arglex_token_files_from:
	    switch (arglex())
	    {
	    default:
		usage();

	    case arglex_token_string:
		slurp(arglex_value.alv_string, filenames);
		break;

	    case arglex_token_stdio:
		slurp(0, filenames);
		break;
	    }
	    break;
	}
	arglex();
    }

    if (!proc)
	proc = new process_tail(print);
    proc->run(filenames);
    delete proc;

    return 0;
}
