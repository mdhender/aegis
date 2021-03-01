//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the checkin class
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/error.h> // hmmm...
#include <common/progname.h>
#include <common/trace.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/undo.h>

#include <ae-repo-ci/arglex3.h>
#include <ae-repo-ci/checkin.h>
#include <ae-repo-ci/repository.h>


void
checkin_usage()
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <filename>\n", progname);
    fprintf(stderr, "       %s --list [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


void
checkin()
{
    trace(("checkin()\n{\n"));
    repository *rp = 0;
    change_identifier cid;
    arglex();
    nstring module;
    nstring directory;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(checkin_usage);
	    continue;

	case arglex_token_project:
	case arglex_token_string:
	case arglex_token_change:
	case arglex_token_number:
	    cid.command_line_parse(checkin_usage);
	    continue;

	case arglex_token_module:
	    if (!module.empty())
		duplicate_option(checkin_usage);
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_module, checkin_usage);
	    module = arglex_value.alv_string;
	    break;

	case arglex_token_directory:
	    if (!directory.empty())
		duplicate_option(checkin_usage);
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, checkin_usage);
	    directory = arglex_value.alv_string;
	    break;

	case arglex_token_repository:
	    if (rp)
		duplicate_option(checkin_usage);
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_repository, checkin_usage);
	    rp = repository::factory(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    if (!rp)
	error_raw("now repository type specified");

    //
    // reject illegal combinations of options
    //
    cid.command_line_check(checkin_usage);

    if (!change_is_completed(cid.get_cp()))
    {
	fatal_raw("change wrong state");
    }
    trace(("change number = %ld\n", cid.get_cp()->number));

    //
    // Default the module name to the name of the project
    // without version (branch) numbers.
    //
    if (module.empty())
    {
	module = nstring(cid.get_pp()->parent_get()->name_get());
    }
    trace(("module = %s\n", module.quote_c().c_str()));

    //
    // create a work area that we can make the duplicate changes to
    //
    os_become_orig();
    rp->checkout(module, directory);
    os_become_undo();

    //
    // apply each file to the work area
    //
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = change_file_nth(cid.get_cp(), j, view_path_first);
	if (!src)
	    break;
	switch (src->action)
	{
	case file_action_create:
	    if (src->move)
	    {
		nstring from(project_file_path(cid.get_pp(), src));
		os_become_orig();
		rp->rename_file
	       	(
		    nstring(src->move),
		    nstring(src->file_name),
		    from
		);
		os_become_undo();
	    }
	    else
	    {
		nstring from(project_file_path(cid.get_pp(), src));
		os_become_orig();
		rp->add_file(nstring(src->file_name), from);
		os_become_undo();
	    }
	    break;

	case file_action_transparent:
	case file_action_modify:
	    {
		nstring from(project_file_path(cid.get_pp(), src));
		os_become_orig();
		rp->modify_file(nstring(src->file_name), from);
		os_become_undo();
	    }
	    break;

	case file_action_remove:
	    if (!src->move)
	    {
		os_become_orig();
		rp->remove_file(nstring(src->file_name));
		os_become_undo();
	    }
	    break;

	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    assert(0);
	    break;
	}
    }

    //
    // commit all the duplicate changes to the duplicate repository
    //
    // FIXME: add some appropriate comment, generated from change brief
    // description, perhapse?
    //
    nstring message(change_version_get(cid.get_cp()));
    os_become_orig();
    rp->commit(message);
    delete rp;
    rp = 0;
    os_become_undo();

    trace(("}\n"));
}
