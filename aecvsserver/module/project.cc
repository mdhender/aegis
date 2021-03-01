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
// MANIFEST: functions to manipulate projects
//

#include <error.h> // for assert
#include <file_info.h>
#include <input/file.h>
#include <module/private.h>
#include <module/project.h>
#include <module/project_bogu.h>
#include <response/created.h>
#include <response/clear_sticky.h>
#include <response/clearstatdir.h>
#include <response/update_exist.h>
#include <os.h>
#include <project/file.h>
#include <project/history.h>
#include <server.h>
#include <symtab.h>


typedef struct module_project_ty module_project_ty;
struct module_project_ty
{
    module_ty       inherited;
    project_ty      *pp;
};


static void
destructor(module_ty *mp)
{
    module_project_ty *mpp;

    mpp = (module_project_ty *)mp;
    project_free(mpp->pp);
    mpp->pp = 0;
}


static void
groan(module_ty *mp, server_ty *sp, const char *request_name)
{
    module_project_ty *mpp;

    mpp = (module_project_ty *)mp;
    server_error
    (
	sp,
	"%s: project \"%s\": you may not alter project files directly, you "
	    "must use an Aegis change instead, and the corresponding module",
	request_name,
	project_name_get(mpp->pp)->str_text
    );
}


static void
modified(module_ty *mp, server_ty *sp, string_ty *file_name, file_info_ty *fip,
    input_ty *ip)
{
    //
    // It is an error to try to write a project file.
    //
    groan(mp, sp, "Modified");
}


static string_ty *
name(module_ty *mp)
{
    module_project_ty *mpp;

    mpp = (module_project_ty *)mp;
    return project_name_get(mpp->pp);
}


static int
update(module_ty *mp, server_ty *sp, string_ty *client_side_0,
    string_ty *server_side_0, module_options_ty *opt)
{
    module_project_ty *mpp;
    size_t          j;

    mpp = (module_project_ty *)mp;

    //
    // Form a list of files by unioning the source files of the project
    // and all the ancestor branches together.
    //
    // For each of these files, send information about their contents.
    //
    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;
	string_ty       *client_side;
	string_ty       *server_side;
	string_ty       *path;
	int             need_to_unlink;
	input_ty        *ip;
	int             mode;
	string_ty       *version;
	file_info_ty    *fip;

	src = project_file_nth(mpp->pp, j, view_path_extreme);
	if (!src)
	    break;
	switch (src->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;
	}
	switch (src->action)
	{
	case file_action_remove:
	    continue;

	case file_action_transparent:
	    assert(0);
	    continue;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    break;
	}

	//
        // Make sure the client creates the directories for us.
	//
	server_side = os_path_cat(module_name(mp), src->file_name);
	if (!is_update_prefix(server_side_0, server_side, opt->d))
	{
	    //
	    // don't create files which are not under one of the
	    // Directories specified by the client.
	    //
	    str_free(server_side);
	    continue;
	}
	client_side = server_directory_calc_client_side(sp, server_side);

	//
	// Make sure the directory exists.
	//
	server_mkdir_above(sp, client_side, server_side);
	server_updating_verbose(sp, client_side);

	//
	// Determine where to get the file from.
	//
	path = project_file_version_path(mpp->pp, src, &need_to_unlink);
	os_become_orig();
	ip = input_file_open(path);
	if (need_to_unlink)
	    input_file_unlink_on_close(ip);
	os_become_undo();
	str_free(path);

	//
	// Determine the file mode.
	//
	mode = 0666;
	if (src->executable)
	    mode |= 0111;
	mode &= ~project_umask_get(mpp->pp);

	//
	// Determine the version string to send to the client.
	// Special cases:
	//	""   - no user file,
	//	"0"  - new user file,
	//	"-"  - user file to be removed
	//
	version = 0;
	switch (src->action)
	{
	case file_action_remove:
	    version = str_from_c("-");
	    break;

	case file_action_transparent:
	    version = str_from_c("0");
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    if (src->edit && src->edit->revision)
		version = str_copy(src->edit->revision);
	    else
		version = str_from_c("0");
	    break;
	}
	if (!version)
	    version = str_from_c("");

	//
	// Queue the response to be sent.
	//
	fip = server_file_info_find(sp, server_side, 0);
	if (!fip)
	{
	    server_response_queue
	    (
		sp,
		response_created_new
		(
		    client_side,
		    server_side,
		    ip,
		    mode,
		    version
		)
	    );
	}
	else if (str_equal(fip->version, version))
	{
	    //
	    // What they copied the first time is still the current
	    // version on the server side.
	    //
	    if (fip->modified > 0)
	    {
		string_ty       *sub;

		//
		// Remind them they need to commit it.
		// Except they can't, bwah ha ha ha.
		//
		sub = os_entryname(client_side);
		server_m(sp, "M %s (oops)\n", sub->str_text);
		str_free(sub);
	    }
	}
	else
	{
	    server_response_queue
	    (
		sp,
		response_update_existing_new
		(
		    client_side,
		    server_side,
		    ip,
		    mode,
		    version
		)
	    );
	}
	str_free(server_side);
	str_free(client_side);
	str_free(version);
    }
    return 1;
}


static int
checkin(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    groan(mp, sp, "ci");
    return 0;
}


static int
add(module_ty *mp, server_ty *sp, string_ty *client_side_0,
    string_ty *server_side_0, module_options_ty *opt)
{
    groan(mp, sp, "add");
    return 0;
}


static int
remove(module_ty *mp, server_ty *sp, string_ty *client_side_0,
    string_ty *server_side_0, module_options_ty *opt)
{
    groan(mp, sp, "remove");
    return 0;
}


static const module_method_ty vtbl =
{
    sizeof(module_project_ty),
    destructor,
    modified,
    name,
    update,
    checkin,
    add,
    remove,
    0, // not bogus
    "project"
};


module_ty *
module_project_new(string_ty *project_name)
{
    project_ty      *pp;
    module_ty       *mp;
    module_project_ty *mpp;

    pp = project_alloc(project_name);
    if (!project_bind_existing_errok(pp))
    {
	project_free(pp);
	return module_project_bogus_new(project_name);
    }

    mp = module_new(&vtbl);
    mpp = (module_project_ty *)mp;
    mpp->pp = pp;
    return mp;
}
