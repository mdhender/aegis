//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/sub.h>
#include <libaegis/tstrslt.h>

#include <aesubunit/arglex3.h>
#include <aesubunit/subunit.h>


void
subunit_usage()
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", prog);
    fprintf(stderr, "       %s [ <option>... ] -Batch <filename>...\n", prog);
    exit(1);
}

enum
{
    result_success,
    result_failure,
    result_no_result
};


static int
run(const nstring &filename)
{
    nstring command;
    if (os_executable(filename.get_ref()))
	command = filename.quote_shell() + " 2>&1";
    else
	command = "sh " + filename.quote_shell() + " 2>&1";
    FILE *fp = popen(command.c_str(), "r");
    if (!fp)
    {
	int errno_old = errno;
	sub_context_ty sc(__FILE__, __LINE__);
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", command);
	sc.fatal_intl(i18n("open $filename: $errno"));
	// NOTREACHED
    }
    error_raw("%s", command.c_str());
    nstring_list lines;
    bool success = false;
    bool failure = false;
    bool quoting = false;
    for (;;)
    {
	char buffer[1 << 13];
	if (!fgets(buffer, sizeof(buffer), fp))
	{
	    if (ferror(fp))
	    {
		int errno_old = errno;
		sub_context_ty sc(__FILE__, __LINE__);
		sc.errno_setx(errno_old);
		sc.var_set_string("File_Name", command);
		sc.fatal_intl(i18n("read $filename: $errno"));
		// NOTREACHED
	    }
	    break;
	}
	if (quoting)
	{
	    if (0 == strcmp(buffer, "]\n"))
	    {
		quoting = false;
	    }
	    else
	    {
		if (buffer[0] == ']' && strchr(buffer, ']'))
		    lines.push_back(nstring(buffer + 1));
		else
		    lines.push_back(buffer);
	    }
	}
	else
	{
	    if (0 == memcmp(buffer, "success", 7))
		success = true;
	    if (0 == memcmp(buffer, "failure", 7))
		success = true;
	    if (strchr(buffer, '['))
		quoting = true;
	}
    }
    int exit_status = pclose(fp);
    if (exit_status)
	return result_no_result;
    if (failure)
	return result_failure;
    if (!success)
	return result_no_result;
    return result_success;
}


static void
run_batch(const nstring_list &filename, const nstring &outfile)
{
    tstrslt_ty *result = (tstrslt_ty *)tstrslt_type.alloc();
    result->test_result =
	(tstrslt_test_result_list_ty *)tstrslt_test_result_list_type.alloc();
    for (size_t j = 0; j < filename.size(); ++j)
    {
	nstring file_name = filename[j];
	int es = run(file_name);

	tstrslt_test_result_ty *mp =
	    (tstrslt_test_result_ty *)tstrslt_test_result_type.alloc();
	mp->file_name = str_copy(file_name.get_ref());
	mp->exit_status = es;

	meta_type *bogus = 0;
	tstrslt_test_result_ty **mpp =
	    (tstrslt_test_result_ty **)
	    tstrslt_test_result_type.list_parse(result->test_result, &bogus);
	assert(bogus == &tstrslt_test_result_type);
	*mpp = mp;
    }
    output::pointer op = output_file::open(outfile, false);
    tstrslt_write(op, result);
    tstrslt_type.free(result);
}


void
subunit()
{
    nstring_list filename;
    bool batch = false;
    const char *outfile = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    bad_argument(subunit_usage);
	    // NOTREACHED

	case arglex_token_batch:
	    if (batch)
		duplicate_option(subunit_usage);
	    batch = true;
	    break;

	case arglex_token_string:
	    filename.push_back(arglex_value.alv_string);
	    break;

	case arglex_token_output:
	    if (outfile)
		duplicate_option(subunit_usage);
	    switch (arglex())
	    {
	    case arglex_token_string:
		outfile = arglex_value.alv_string;
		break;

	    case arglex_token_stdio:
		outfile = "";
		break;

	    default:
		option_needs_file(arglex_token_output, subunit_usage);
		// NOTREACHED
	    }
	    break;
	}
	arglex();
    }
    if (filename.empty())
	fatal_intl(0, i18n("no file names"));
    if (batch)
    {
	if (!outfile)
	    outfile = "";
	run_batch(filename, outfile);
	quit(0);
    }
    if (filename.size() > 1)
	fatal_intl(0, i18n("too many files"));
    int n = run(filename[0]);
    if (n)
	quit(n);
}
