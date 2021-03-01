//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the diff class
//

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex3.h>
#include <arglex/project.h>
#include <change_id.h>
#include <diff.h>
#include <file_version.h>
#include <help.h>
#include <nstring_list.h>
#include <option.h>
#include <os.h>
#include <progname.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
diff_usage()
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", progname);
    fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


void
diff()
{
    trace(("diff()\n{\n"));
    nstring base_command("diff");
    string_ty *project_name = 0;
    nstring filename;
    change_id first;
    change_id second;
    int context = 0;
    int unified = 0;
    bool text = false;
    bool ignore_all_space = false;
    bool ignore_blank_lines = false;
    bool ignore_case = false;
    bool ignore_space_change = false;
    bool show_c_function = false;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(diff_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, diff_usage);
	    first.set_project_name(project_name);
	    second.set_project_name(project_name);
	    continue;

	case arglex_token_baseline:
	case arglex_token_branch:
	case arglex_token_change:
	case arglex_token_delta:
	case arglex_token_delta_date:
	case arglex_token_delta_from_change:
	case arglex_token_grandparent:
	case arglex_token_number:
	case arglex_token_trunk:
	    if (!first.set())
		first.command_line_parse(diff_usage);
	    else
		second.command_line_parse(diff_usage);
	    continue;

	case arglex_token_string:
	    if (filename)
		fatal_too_many_files();
	    filename = arglex_value.alv_string;
	    break;

	case arglex_token_context:
	    if (context)
		duplicate_option(diff_usage);
	    if (arglex() != arglex_token_number)
	    {
		context = 3;
		continue;
	    }
	    context = arglex_value.alv_number;
	    if (context < 1)
		context = 1;
	    break;

	case arglex_token_unified:
	    if (unified)
		duplicate_option(diff_usage);
	    if (arglex() != arglex_token_number)
	    {
		unified = 3;
		continue;
	    }
	    unified = arglex_value.alv_number;
	    if (unified < 1)
		unified = 1;
	    break;

	case arglex_token_text:
	    if (text)
		duplicate_option(diff_usage);
	    text = true;
	    break;

	case arglex_token_ignore_all_space:
	    if (ignore_all_space)
		duplicate_option(diff_usage);
	    ignore_all_space = true;
	    break;

	case arglex_token_ignore_blank_lines:
	    if (ignore_blank_lines)
		duplicate_option(diff_usage);
	    ignore_blank_lines = true;
	    break;

	case arglex_token_ignore_case:
	    if (ignore_case)
		duplicate_option(diff_usage);
	    ignore_case = true;
	    break;

	case arglex_token_ignore_space_change:
	    if (ignore_space_change)
		duplicate_option(diff_usage);
	    ignore_space_change = true;
	    break;

	case arglex_token_show_c_function:
	    if (show_c_function)
		duplicate_option(diff_usage);
	    show_c_function = true;
	    break;
	}
	arglex();
    }
    if (context && unified)
    {
	mutually_exclusive_options
	(
	    arglex_token_context,
	    arglex_token_unified,
	    diff_usage
	);
    }
    if (!filename)
    {
	error_intl(0, i18n("too few files named"));
	diff_usage();
    }

    //
    // Default a few things if they gave zero or one change, rather than two.
    //
    if (!first.set() && !second.set())
	first.set_baseline();

    //
    // reject illegal combinations of options
    //
    first.command_line_check(diff_usage);
    second.command_line_check(diff_usage);

    //
    // locate the project and branch data
    //
    first.set_project();
    second.set_project();

    //
    // Get the two versions of the file.
    //
    first.set_change(diff_usage);
    file_version lhs = first.get_file_version(filename);
    trace(("lhs=%s\n", lhs.get_path().c_str()));
    second.set_change(diff_usage);
    file_version rhs = second.get_file_version(filename);
    trace(("rhs=%s\n", rhs.get_path().c_str()));

    //
    // Build the command to be executed.
    //
    nstring_list command_args;
    command_args.push_back(base_command);
    if (text)
	command_args.push_back("--text");
    if (context)
	command_args.push_back(nstring::format("-C%d", context));
    else if (unified)
	command_args.push_back(nstring::format("-U%d", unified));
    if (ignore_all_space)
	command_args.push_back("--ignore-all-space");
    if (ignore_blank_lines)
	command_args.push_back("--ignore-blank-lines");
    if (ignore_case)
	command_args.push_back("--ignore-case");
    if (ignore_space_change)
	command_args.push_back("--ignore-space-change");
    if (show_c_function)
	command_args.push_back("--show-c-function");
    command_args.push_back(lhs.get_path().quote_shell());
    command_args.push_back(rhs.get_path().quote_shell());

    nstring command = command_args.unsplit(" ");

    //
    // Execute the command.
    //
    trace(("command = \"%s\"\n", command.c_str()));
    int flags = OS_EXEC_FLAG_NO_INPUT;
    if (!option_verbose_get())
	flags |= OS_EXEC_FLAG_SILENT;
    os_become_orig();
    os_execute(command.get_ref(), flags, os_curdir());
    os_become_undo();
    trace(("}\n"));
}
