//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller;
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License,
//      version 3, as published by the Free Software Foundation.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/unistd.h>

#include <common/env.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/safe_freopen.h>
#include <libaegis/version.h>
#include <libaegis/view_path.h>

#include <aemakegen/target.h>

enum
{
    arglex_token_script = ARGLEX2_MAX,
    arglex_token_search_path,
    arglex_token_target,
    ARGLEX3_MAX
};

static arglex_table_ty argtab[] =
{
    { "-SCRipt", arglex_token_script, },
    { "-Search_Path", arglex_token_search_path, },
    { "-TArget", arglex_token_target, },
    ARGLEX_END_MARKER
};


static void
usage()
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>...\n", prog);
    fprintf(stderr, "       %s --version\n", prog);
    quit(1);
}


static void
main_help()
{
    help(0, usage);
}


int
main(int argc, char **argv)
{
    resource_limits_init();
    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    switch (arglex())
    {
    case arglex_token_help:
	main_help();
	quit(0);

    case arglex_token_version:
	version();
	quit(0);
    }

    change_identifier *cid = 0;

    nstring_list files;
    nstring tname;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	case arglex_token_search_path:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_search_path, usage);
	    target::vpath(arglex_value.alv_string);
	    break;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_delta_from_change:
        case arglex_token_delta_name:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
            if (!cid)
                cid = new change_identifier();
            cid->command_line_parse(usage);
            continue;

	case arglex_token_script:
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_script, usage);
	    target::script(arglex_value.alv_string);
	    break;

	case arglex_token_string:
	    files.push_back(arglex_value.alv_string);
	    break;

	case arglex_token_target:
	    if (arglex() != arglex_token_string)
                option_needs_string(arglex_token_target, usage);
            tname = arglex_value.alv_string;
	    break;

        case arglex_token_output:
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_output, usage);
            safe_freopen(arglex_value.alv_string, "w", stdout);
            break;

	default:
	    usage();
	}
	arglex();
    }

    //
    // If the user mentioned a project or a change number on the command
    // line, extract the necessary information from the change.
    //
    if (cid)
    {
        change::pointer cp = cid->get_cp();
        for (size_t j = 0; ; ++j)
        {
            fstate_src_ty *src = change_file_nth(cp, j, view_path_extreme);
            if (!src)
                break;
            files.push_back(nstring(src->file_name));
        }

        string_list_ty dirs;
        change_search_path_get(cp, &dirs, 1);
        for (size_t k = 0; k < dirs.size(); ++k)
	    target::vpath(nstring(dirs[k]));
    }

    if (files.empty())
	usage();
    files.sort();

    target *tp = target::create(tname);
    tp->process(files);

    return 0;
}
