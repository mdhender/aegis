//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <libaegis/change/identifier.h>
#include <libaegis/fstate.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <ae-repo-ci/repository.h>


void
repository::file_wrapper(change_identifier &cid, fstate_src_ty *src)
{
    switch (src->action)
    {
    case file_action_create:
	if (!src->move)
	    add_file_wrapper(cid, src);
	else
	    rename_file_wrapper(cid, src);
	break;

    case file_action_transparent:
    case file_action_modify:
	modify_file_wrapper(cid, src);
	break;

    case file_action_remove:
	if (!src->move)
	    remove_file_wrapper(cid, src);
	break;

    case file_action_insulate:
#ifndef DEBUG
    default:
#endif
	assert(0);
	break;
    }
}


void
repository::add_file_wrapper(change_identifier &cid, fstate_src_ty *src)
{
    //
    // If the file already exists in the repository,
    // we have to substitute a modify action instead.
    //
    nstring file_name(src->file_name);
    os_become_orig();
    bool exists = os_exists(os_path_join(get_directory(), file_name));
    os_become_undo();
    if (exists)
    {
	modify_file_wrapper(cid, src);
	return;
    }

    //
    // Obtain the necessary version of the file.
    //
    int need_unlink = 0;
    nstring path(project_file_version_path(cid.get_pp(), src, &need_unlink));

    //
    // Run the actual repository "add" command.
    //
    os_become_orig();
    add_file(file_name, path);
    if (need_unlink)
	os_unlink_errok(path);
    os_become_undo();
}


void
repository::modify_file_wrapper(change_identifier &cid, fstate_src_ty *src)
{
    //
    // If the file does not exist in the repository,
    // we have to substitute an add action instead.
    //
    nstring file_name(src->file_name);
    os_become_orig();
    bool exists = os_exists(os_path_join(get_directory(), file_name));
    os_become_undo();
    if (!exists)
    {
	add_file_wrapper(cid, src);
	return;
    }

    //
    // Obtain the necessary version of the file.
    //
    int need_unlink = 0;
    nstring path(project_file_version_path(cid.get_pp(), src, &need_unlink));

    //
    // Run the actual repository "modify" command.
    //
    os_become_orig();
    modify_file(file_name, path);
    if (need_unlink)
	os_unlink_errok(path);
    os_become_undo();
}


void
repository::remove_file_wrapper(change_identifier &, fstate_src_ty *src)
{
    //
    // If the file does not exist in the repository,
    // don't do anything.
    //
    nstring file_name(src->file_name);
    os_become_orig();
    bool exists = os_exists(os_path_join(get_directory(), file_name));
    os_become_undo();
    if (!exists)
	return;

    //
    // Run the actual repository "remove" command.
    //
    os_become_orig();
    remove_file(file_name);
    os_become_undo();
}


void
repository::rename_file_wrapper(change_identifier &cid, fstate_src_ty *src)
{
    //
    // If the file does not exist in the repository,
    // don't do anything.
    //
    assert(src->action == file_action_create);
    nstring from_file_name(src->move);
    nstring to_file_name(src->file_name);
    os_become_orig();
    bool from_exists = os_exists(os_path_join(get_directory(), from_file_name));
    bool to_exists = os_exists(os_path_join(get_directory(), to_file_name));
    os_become_undo();

    if (from_exists)
    {
	if (to_exists)
	{
	    os_become_orig();
	    remove_file(from_file_name);
	    os_become_undo();

	    modify_file_wrapper(cid, src);
	    return;
	}
	else
	{
	    //
	    // Obtain the necessary version of the file.
	    //
	    int need_unlink = 0;
	    nstring path
		(
		    project_file_version_path(cid.get_pp(), src, &need_unlink)
		);

	    //
	    // Run the actual repository "rename" command.
	    //
	    os_become_orig();
	    rename_file(from_file_name, to_file_name, path);
	    os_become_undo();
	}
    }
    else
    {
	// source does NOT exist!
	if (to_exists)
	{
	    // No source, but destination exists
	    modify_file_wrapper(cid, src);
	    return;
	}
	else
	{
	    // No source, no destination.
	    add_file_wrapper(cid, src);
	}
    }
}
