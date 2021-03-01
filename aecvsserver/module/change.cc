//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate changes
//

#include <ac/string.h>

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/lock_sync.h>
#include <cstate.h>
#include <error.h> // for assert
#include <fake_version.h>
#include <file_info.h>
#include <fstate.h>
#include <input/file.h>
#include <module/change.h>
#include <module/change_bogus.h>
#include <module/private.h>
#include <module/project_bogu.h>
#include <os.h>
#include <output/file.h>
#include <project.h>
#include <project/file.h>
#include <response/checked_in.h>
#include <response/created.h>
#include <response/new_entry.h>
#include <response/removed.h>
#include <response/remove_entry.h>
#include <response/update_exist.h>
#include <symtab.h>
#include <user.h>


struct module_change_ty
{
    module_ty       inherited;
    change_ty       *cp;
    project_ty      *pp;
    user_ty         *up;
};


static void
destructor(module_ty *mp)
{
    module_change_ty *mcp;

    mcp = (module_change_ty *)mp;
    change_free(mcp->cp);
    mcp->cp = 0;
    user_free(mcp->up);
    mcp->up = 0;
    project_free(mcp->pp);
    mcp->pp = 0;
}


static void
modified(module_ty *mp, server_ty *sp, string_ty *file_name, file_info_ty *fip,
    input_ty *contents)
{
    module_change_ty *mcp;
    output_ty       *op;
    fstate_src_ty   *csrc;
    string_ty       *dd;
    string_ty       *abs_file_name;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    mcp = (module_change_ty *)mp;
    if (!change_is_being_developed(mcp->cp))
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: this change	must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
	return;
    }

    //
    // It is an error if the change is actually a branch.
    //
    if (change_was_a_branch(mcp->cp))
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: is a branch; to modify "
		"files you must use a change on the branch",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
	return;
    }

    //
    // It is an error if the change is not assigned to the current user.
    //
    if (!str_equal(change_developer_name(mcp->cp), user_name(mcp->up)))
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: is owned by user \"%s\", "
		"but it must be owned by user \"%s\" for this to work",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number,
	    change_developer_name(mcp->cp)->str_text,
	    user_name(mcp->up)->str_text
	);
    }

    //
    // If the file doesn't exist in the change, copy it or create it
    // as appropriate.
    //
    csrc = change_file_find(mcp->cp, file_name, view_path_first);
    if (!csrc)
    {
	string_ty       *qp;
	string_ty       *qf;
	const char      *verb;
	string_ty       *the_command;
	int             ok;

	qp = str_quote_shell(project_name_get(mcp->pp));
	qf = str_quote_shell(file_name);
	if (!project_file_find(mcp->pp, file_name, view_path_extreme))
	{
	    //
	    // FIXME: what if it's a new test?
	    // (aepatch has the same problem)
	    //
	    verb = "-new-file";
	}
	else
	{
	    verb = "-copy-file";
	}
	the_command =
	    str_format
	    (
		"aegis %s -project=%s -change=%ld -base-relative -v %s",
		verb,
		qp->str_text,
		mcp->cp->number,
		qf->str_text
	    );
	str_free(qp);
	str_free(qf);

	//
	// Run the command.
	// if there is a problem, it will also send the error to the client.
	//
	ok = server_execute(sp, the_command);
	str_free(the_command);
	if (!ok)
	    return;

	//
        // Change objects can be very long lived in the aecvsserver,
        // so make sure that we re-read the meta data soon.
	//
	change_lock_sync_forced(mcp->cp);

	//
	// Now re-get the file information.
	//
	csrc = change_file_find(mcp->cp, file_name, view_path_first);
	assert(csrc);
    }
    else if (csrc->action == file_action_remove)
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: file \"%s\" is being "
		"removed, it makes no sense to say it's been modified",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number,
	    file_name->str_text
	);
	return;
    }

    //
    // Figure the path of the file to be written.
    //
    dd = change_development_directory_get(mcp->cp, 0);
    abs_file_name = os_path_cat(dd, file_name);

    //
    // Normalize the mode (Aegis has its own idea about file modes).
    //
    if (fip->mode & 0222)
	fip->mode |= 0222;
    if (fip->mode & 0111)
	fip->mode |= 0111;
    fip->mode |= 0644;
    fip->mode &= ~change_umask(mcp->cp);

    //
    // Copy the file contents to their destination.
    //
    os_become_orig();
    op = output_file_binary_open(abs_file_name);
    input_to_output(contents, op);
    delete op;

    //
    // And make sure it is in the specified mode.
    //
    os_chmod(abs_file_name, fip->mode);
    os_become_undo();
    str_free(abs_file_name);
}


static string_ty *
canonical_name(module_ty *mp)
{
    module_change_ty *mcp;

    // FIXME: memory leak
    mcp = (module_change_ty *)mp;
    return
	str_format
	(
	    "%s.C%3.3ld",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
}


static int
update(module_ty *mp, server_ty *sp, string_ty *client_side_0,
    string_ty *server_side_0, module_options_ty *opt)
{
    module_change_ty *mcp;
    size_t          j;
    static string_ty *minus;
    static string_ty *zero;

    mcp = (module_change_ty *)mp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    // FIXME: actually, it's OK if the file is between being_developed
    // and being_integrated, but only read-only for the later ones.
    //
    // FIXME: what about changes in the completed state, we will need
    // to use project_file_roll_forward instead.
    //
    mcp = (module_change_ty *)mp;
    if (!change_is_being_developed(mcp->cp))
    {
	server_error
	(
	    sp,
	    "project \"%s\": change %ld: this change must be in the "
		"\"being_developed\" state for this to work",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
	return 0;
    }

    //
    // Form a list of files by unioning the source files of the change
    // and all the ancestor branches together.
    //
    // For each of these files (that the client wants to know about)
    // send information about their contents.
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
	int             is_local;
	file_info_ty    *fip;

	src = change_file_nth(mcp->cp, j, view_path_simple);
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
	server_mkdir_above(sp, client_side, server_side);
	server_updating_verbose(sp, client_side);

	//
	// Determine where to get the file from.
	//
	version = 0;
	path = change_file_version_path(mcp->cp, src, &need_to_unlink);
	is_local = !!change_file_find(mcp->cp, src->file_name, view_path_first);
	os_become_orig();
	ip = input_file_open(path);
	if (need_to_unlink)
	    input_file_unlink_on_close(ip);
	if (is_local && os_executable(path))
	    src->executable = true;
	if (is_local)
	    version = fake_version(os_mtime_actual(path));
	os_become_undo();
	str_free(path);

	//
	// Determine the file mode.
	//
	mode = 0666;
	if (src->executable)
	    mode |= 0111;
	mode &= ~change_umask(mcp->cp);

	//
	// Determine the version string to send to the client.
	// Special cases:
	//	""   - no user file,
	//	"0"  - new user file,
	//	"-"  - user file to be removed
	//
	if (!version)
	{
	    switch (src->action)
	    {
	    case file_action_remove:
		//
                // What do do depends on whether the file exists on the
                // client or not.
		//
		// the client has never heard of it
		//     do nothing
		//
		// The client has no Entry for it, but said Questionable
		//     do nothing
		//
		// the client Entry has a removed version ("-")
		//     do nothing.
		//
		// the client Entry has a new file version ("0")
		//     do nothing.
		//
		// the client says it exists
		//     if Is-modified
		//         send the new file ("0") Entry
		//     otherwise
		//         send the Removed response, which will remove
		//         the entry and the file from the client side.
		//
	        fip = server_file_info_find(sp, server_side, 0);
		if (!fip || !fip->version)
		    goto do_nothing;
		if (!minus)
		    minus = str_from_c("-");
		if (!zero)
		    zero = str_from_c("0");
		if
		(
		    str_equal(fip->version, minus)
		||
		    str_equal(fip->version, zero)
		)
		    goto do_nothing;
		if (fip->modified > 0)
		{
		    version = str_copy(zero);
		    break;
		}
		server_response_queue
		(
		    sp,
		    response_removed_new(client_side, server_side)
		);
		goto do_nothing;

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
	}

	fip = server_file_info_find(sp, server_side, 0);
	if (!fip)
	{
	    //
	    // Queue the response to be sent.
	    //
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
	else if (opt->C)
	{
	    //
	    // We have been told to over-write what they have on the
	    // client side.
	    //
	    if (fip->modified > 0)
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
		//
		sub = os_entryname(client_side);
		server_m(sp, "M %s\n", sub->str_text);
		str_free(sub);
	    }
	}
	else
	{
	    //
	    // What they copied the first time is no longer the
	    // current version on the server side.
	    //
	    if (fip->modified)
	    {
		//
                // The server side change and the client side changed.
                // The Modified request (received earlier) has copied
                // the file into the change, and the over-written it.
		//
                // We have to send the file back to them,
                // AFTER running a merge command.
		//
		// FIXME: run the merge command
		//
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
	    else
	    {
		//
                // The client side is out-of-date, resend the file and
                // over-write the client-side contents.
		//
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
	}
    do_nothing:
	str_free(client_side);
	str_free(server_side);
	str_free(version);
    }
    return 1;
}


static int
file_being_deleted(server_ty *sp, string_ty *server_side)
{
    file_info_ty    *fip;
    static string_ty *minus;

    fip = server_file_info_find(sp, server_side, 0);
    if (!fip)
	return 0;
    if (!fip->version)
	return 0;
    if (!minus)
	minus = str_from_c("-");
    return str_equal(fip->version, minus);
}


static int
checkin(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    module_change_ty *mcp;
    fstate_src_ty   *src;
    int             mode;
    string_ty       *version;
    string_ty       *filename;
    const char      *strp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    mcp = (module_change_ty *)mp;
    if (!change_is_being_developed(mcp->cp))
    {
	server_error
	(
	    sp,
	    "ci: project \"%s\": change %ld: this change must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
	return 0;
    }

    //
    // Extract the baseline-relative name of the file.
    //
    // The server-side string sent by the client will be
    // ROOT_PATH/project.Cnnn/filename so skip three slashes (ROOT_PATH
    // starts with a slash) and use the rest.  Except that the Directory
    // request code strips off the ROOT_PATH/ part, so skip one slash
    // instead.
    //
    strp = strchr(server_side->str_text, '/');
    assert(strp);
    filename = str_from_c(strp ? strp + 1 : ".");

    src = change_file_find(mcp->cp, filename, view_path_first);
    if (!src)
    {
	if (file_being_deleted(sp, server_side))
	{
	    string_ty       *qp;
	    string_ty       *qf;
	    string_ty       *the_command;
	    int             ok;

	    qp = str_quote_shell(project_name_get(mcp->pp));
	    qf = str_quote_shell(filename);
	    the_command =
		str_format
		(
	  "aegis --remove-file --project=%s --change=%ld --base-relative -v %s",
		    qp->str_text,
		    mcp->cp->number,
		    qf->str_text
		);
	    str_free(qp);
	    str_free(qf);

	    //
            // Run the command.  If there is a problem, it will also
            // send the error to the client.
	    //
	    ok = server_execute(sp, the_command);
	    str_free(the_command);
	    if (!ok)
		return 0;

	    //
            // Change objects can be very long lived in the aecvsserver,
            // so make sure that we re-read the meta data soon.
            //
            change_lock_sync_forced(mcp->cp);

	    //
	    // Let the client know the file is well and truly gone.
            // (We dont use the Removed response because the file is
            // already deleted from thwe client.)
	    //
	    server_response_queue
	    (
		sp,
		response_remove_entry_new(client_side, server_side)
	    );

	    //
	    // Report success.
	    //
	    return 1;
	}

	server_error
	(
	    sp,
	    "ci: project \"%s\": change %ld: file \"%s\" unknown",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number,
	    filename->str_text
	);
	return 0;
    }

    //
    // For changes which are being developeed, like this one, we
    // have to look at the file itself for the executable bit.
    //
    if (src->action != file_action_remove)
    {
	string_ty       *path;

	path = change_file_path(mcp->cp, filename);
	os_become_orig();
	if (os_executable(path))
	    src->executable = true;
	os_become_undo();
	str_free(path);
    }

    //
    // Determine the file mode.
    //
    mode = 0666;
    if (src->executable)
	mode |= 0111;
    mode &= ~change_umask(mcp->cp);

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
    server_response_queue
    (
	sp,
	response_checked_in_new(client_side, server_side, mode, version)
    );
    str_free(version);

    return 1;
}


static int
add(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt)
{
    module_change_ty *mcp;
    fstate_src_ty   *src;
    int             mode;
    string_ty       *version;
    string_ty       *filename;
    const char      *strp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    mcp = (module_change_ty *)mp;
    if (!change_is_being_developed(mcp->cp))
    {
	server_error
	(
	    sp,
	    "add: project \"%s\": change %ld: this change must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
	return 0;
    }

    //
    // Extract the baseline-relative name of the file.
    //
    // The server-side string sent by the client will be
    // ROOT_PATH/project.Cnnn/filename so skip three slashes and use
    // the rest.  Except that the Directory request code strips off
    // the ROOT_PATH/ part, so skip one slash instead.
    //
    strp = strchr(server_side->str_text, '/');
    assert(strp);
    filename = str_from_c(strp ? strp + 1 : ".");

    //
    // The client sends an "Is-modified" request, so the file isn't
    // necesarily created yet.
    //
    src = change_file_find(mcp->cp, filename, view_path_first);
    if (src)
    {
	//
        // The CVS documentation says that a "cvs add" of a removed file
        // will re-instate it.
	//
	if (src->action == file_action_remove)
	{
	    string_ty       *qp;
	    string_ty       *qf;
	    string_ty       *the_command;
	    int             ok;

	    qp = str_quote_shell(project_name_get(mcp->pp));
	    qf = str_quote_shell(filename);
	    the_command =
		str_format
		(
		    "aegis -rmu -project=%s -change=%ld -base-relative -v %s",
		    qp->str_text,
		    mcp->cp->number,
		    qf->str_text
		);

	    //
	    // Run the command.
	    // if there is a problem, it will also send the error to the client.
	    //
	    ok = server_execute(sp, the_command);
	    str_free(the_command);
	    if (!ok)
	    {
		str_free(qp);
		str_free(qf);
		return 0;
	    }

	    //
	    // Change objects can be very long lived in the aecvsserver,
	    // so make sure that we re-read the meta data soon.
	    //
	    change_lock_sync_forced(mcp->cp);

	    //
	    // Now re-get the file information.
	    //
	    src = project_file_find(mcp->pp, filename, view_path_extreme);
	    if (src)
	    {
		the_command =
		    str_format
		    (
		"aegis -copy-file -project=%s -change=%ld -base-relative -v %s",
			qp->str_text,
			mcp->cp->number,
			qf->str_text
		    );

		//
                // Run the command.  if there is a problem, it will also
                // send the error to the client.
		//
		ok = server_execute(sp, the_command);
		str_free(the_command);
		if (!ok)
		{
		    str_free(qp);
		    str_free(qf);
		    return 0;
		}

		//
		// Change objects can be very long lived in the aecvsserver,
		// so make sure that we re-read the meta data soon.
		//
		change_lock_sync_forced(mcp->cp);

		//
		// Now re-get the file information.
		//
		src = change_file_find(mcp->cp, filename, view_path_first);
		assert(src);
	    }
	    str_free(qp);
	    str_free(qf);
	}
    }

    //
    // For changes which are being developeed, like this one, we
    // have to look at the file itself for the executable bit.
    //
    if (src && src->action != file_action_remove)
    {
	string_ty       *path;

	path = change_file_path(mcp->cp, filename);
	os_become_orig();
	if (os_executable(path))
	    src->executable = true;
	os_become_undo();
	str_free(path);
    }

    //
    // Determine the file mode.
    //
    mode = 0666;
    if (src && src->executable)
	mode |= 0111;
    mode &= ~change_umask(mcp->cp);

    if (!src)
    {
	version = str_from_c("0");
	server_e
	(
	    sp,
	    "scheduling file `%s' for addition",
	    client_side->str_text
	);
	server_response_queue
	(
	    sp,
	    response_new_entry_new(client_side, server_side, mode, version)
	);
	str_free(version);

	return 1;
    }

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
    case file_action_create:
	version = str_from_c("0");
	break;

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
    server_e
    (
	sp,
	"scheduling file `%s' for addition",
	src->file_name->str_text
    );
    server_response_queue
    (
	sp,
	response_new_entry_new(client_side, server_side, mode, version)
    );
    str_free(version);

    return 1;
}


static int
remove(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt)
{
    module_change_ty *mcp;
    fstate_src_ty   *src;
    int             mode;
    string_ty       *version;
    string_ty       *filename;
    const char      *strp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    mcp = (module_change_ty *)mp;
    if (!change_is_being_developed(mcp->cp))
    {
	server_error
	(
	    sp,
	    "remove: project \"%s\": change %ld: this change must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number
	);
	return 0;
    }

    //
    // Extract the baseline-relative name of the file.
    //
    // The server-side string sent by the client will be
    // ROOT_PATH/project.Cnnn/filename so skip three slashes and use
    // the rest.  Except that the Directory request code strips off
    // the ROOT_PATH/ part, so skip one slash instead.
    //
    strp = strchr(server_side->str_text, '/');
    assert(strp);
    filename = str_from_c(strp ? strp + 1 : ".");

    //
    // If the file already exists in the change,
    // we may have to undo that.
    //
    src = change_file_find(mcp->cp, filename, view_path_first);
    if (src)
    {
	string_ty       *qp;
	string_ty       *qf;
	string_ty       *the_command;
	int             ok;
	const char      *verb;

	verb = "--copy-file-undo";
	switch (src->action)
	{
	case file_action_create:
	    verb = "--new-file-undo";
	    switch (src->usage)
	    {
	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
		break;

	    case file_usage_test:
	    case file_usage_manual_test:
		verb = "--new-test-undo";
		break;
	    }
	    break;

	case  file_action_modify:
	case  file_action_insulate:
	    break;

	case file_action_transparent:
	    verb = "--make-transparent-undo";
	    break;

	case file_action_remove:
	    goto already_being_removed;
	}

	qp = str_quote_shell(project_name_get(mcp->pp));
	qf = str_quote_shell(filename);
	the_command =
	    str_format
	    (
		"aegis %s -project=%s -change=%ld -base-relative -v %s",
		verb,
		qp->str_text,
		mcp->cp->number,
		qf->str_text
	    );
	str_free(qp);
	str_free(qf);

	//
	// Run the command.
	// if there is a problem, it will also send the error to the client.
	//
	ok = server_execute(sp, the_command);
	str_free(the_command);
	if (!ok)
	    return 0;

	//
	// Change objects can be very long lived in the aecvsserver,
	// so make sure that we re-read the meta data soon.
	//
	change_lock_sync_forced(mcp->cp);

	switch (src->action)
	{
	case file_action_create:
	    server_response_queue
	    (
		sp,
		response_remove_entry_new(client_side, server_side)
	    );
	    return 1;

	case file_action_remove:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
	src = 0;
    }

    src = project_file_find(mcp->pp, filename, view_path_extreme);
    if (!src)
    {
	server_error
	(
	    sp,
	    "project \"%s\": change %ld: file \"%s\" cannot be removed "
		"because it does not exist in the project",
	    project_name_get(mcp->pp)->str_text,
	    mcp->cp->number,
	    filename->str_text
	);
	return 0;
    }

    //
    // Determine the version string to send to the client.
    // Special cases:
    //	""   - no user file,
    //	"0"  - new user file,
    //	"-"  - user file to be removed
    //
    already_being_removed:
    version = str_from_c("-");;
    mode = 0644;

    //
    // Queue the response to be sent.
    //
    server_e
    (
	sp,
	"scheduling file `%s' for removal",
	src->file_name->str_text
    );
    server_response_queue
    (
	sp,
	response_new_entry_new(client_side, server_side, mode, version)
    );
    str_free(version);

    return 1;
}


static const module_method_ty vtbl =
{
    sizeof(module_change_ty),
    destructor,
    modified,
    canonical_name,
    update,
    checkin,
    add,
    remove,
    0, // not bogus
    "change"
};


module_ty *
module_change_new(string_ty *project_name, long change_number)
{
    project_ty      *pp;
    change_ty       *cp;
    module_ty       *mp;
    module_change_ty *mcp;

    pp = project_alloc(project_name);
    if (!project_bind_existing_errok(pp))
    {
	project_free(pp);
	return module_project_bogus_new(project_name);
    }
    cp = change_alloc(pp, change_number);
    if (!change_bind_existing_errok(cp))
    {
	change_free(cp);
	project_free(pp);
	return module_change_bogus_new(project_name, change_number);
    }

    mp = module_new(&vtbl);
    mcp = (module_change_ty *)mp;
    mcp->pp = pp;
    mcp->up = user_executing(mcp->pp);
    mcp->cp = cp;
    return mp;
}
