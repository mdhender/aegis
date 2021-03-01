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

#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/symtab.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/lock_sync.h>
#include <libaegis/cstate.h>
#include <libaegis/fstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/user.h>

#include <aecvsserver/fake_version.h>
#include <aecvsserver/file_info.h>
#include <aecvsserver/module/change_bogus.h>
#include <aecvsserver/module/change.h>
#include <aecvsserver/module/project_bogu.h>
#include <aecvsserver/response/checked_in.h>
#include <aecvsserver/response/created.h>
#include <aecvsserver/response/new_entry.h>
#include <aecvsserver/response/removed.h>
#include <aecvsserver/response/remove_entry.h>
#include <aecvsserver/response/update_exist.h>
#include <aecvsserver/server.h>


module_change::~module_change()
{
    change_free(cp);
    cp = 0;
    project_free(pp);
    pp = 0;
}


module_change::module_change(change::pointer arg) :
    cp(arg),
    pp(arg->pp),
    up(user_ty::create())
{
}


void
module_change::modified(server_ty *sp, string_ty *file_name, file_info_ty *fip,
    input &contents)
{
    //
    // It is an error if the change is not in the "being developed" state.
    //
    if (!cp->is_being_developed())
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: this change	must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(pp)->str_text,
	    cp->number
	);
	return;
    }

    //
    // It is an error if the change is actually a branch.
    //
    if (change_was_a_branch(cp))
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: is a branch; to modify "
		"files you must use a change on the branch",
	    project_name_get(pp)->str_text,
	    cp->number
	);
	return;
    }

    //
    // It is an error if the change is not assigned to the current user.
    //
    if (nstring(change_developer_name(cp)) != up->name())
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: is owned by user \"%s\", "
		"but it must be owned by user \"%s\" for this to work",
	    project_name_get(pp)->str_text,
	    cp->number,
	    change_developer_name(cp)->str_text,
	    up->name().c_str()
	);
    }

    //
    // If the file doesn't exist in the change, copy it or create it
    // as appropriate.
    //
    fstate_src_ty *csrc = change_file_find(cp, file_name, view_path_first);
    if (!csrc)
    {
	string_ty       *qp;
	string_ty       *qf;
	const char      *verb;
	string_ty       *the_command;
	int             ok;

	qp = str_quote_shell(project_name_get(pp));
	qf = str_quote_shell(file_name);
	if (!project_file_find(pp, file_name, view_path_extreme))
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
		cp->number,
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
	change_lock_sync_forced(cp);

	//
	// Now re-get the file information.
	//
	csrc = change_file_find(cp, file_name, view_path_first);
	assert(csrc);
    }
    else if (csrc->action == file_action_remove)
    {
	server_error
	(
	    sp,
	    "Modified: project \"%s\": change %ld: file \"%s\" is being "
		"removed, it makes no sense to say it's been modified",
	    project_name_get(pp)->str_text,
	    cp->number,
	    file_name->str_text
	);
	return;
    }

    //
    // Figure the path of the file to be written.
    //
    string_ty *dd = change_development_directory_get(cp, 0);
    string_ty *abs_file_name = os_path_cat(dd, file_name);

    //
    // Normalize the mode (Aegis has its own idea about file modes).
    //
    if (fip->mode & 0222)
	fip->mode |= 0222;
    if (fip->mode & 0111)
	fip->mode |= 0111;
    fip->mode |= 0644;
    fip->mode &= ~change_umask(cp);

    //
    // Copy the file contents to their destination.
    //
    os_become_orig();
    output::pointer op = output_file::binary_open(abs_file_name);
    op << contents;
    op.reset();

    //
    // And make sure it is in the specified mode.
    //
    os_chmod(abs_file_name, fip->mode);
    os_become_undo();
    str_free(abs_file_name);
}


string_ty *
module_change::calculate_canonical_name()
    const
{
    // FIXME: memory leak
    return
	str_format
	(
	    "%s.C%3.3ld",
	    project_name_get(pp)->str_text,
	    cp->number
	);
}


bool
module_change::update(server_ty *sp, string_ty *, string_ty *server_side_0,
    const options &opt)
{
    size_t          j;
    static string_ty *minus_str;
    static string_ty *zero;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    // FIXME: actually, it's OK if the file is between being_developed
    // and being_integrated, but only read-only for the later ones.
    //
    // FIXME: what about changes in the completed state, we will need
    // to use project_file_roll_forward instead.
    //
    if (!cp->is_being_developed())
    {
	server_error
	(
	    sp,
	    "project \"%s\": change %ld: this change must be in the "
		"\"being_developed\" state for this to work",
	    project_name_get(pp)->str_text,
	    cp->number
	);
	return false;
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
	int             mode;
	string_ty       *version;
	int             is_local;
	file_info_ty    *fip;

	src = change_file_nth(cp, j, view_path_simple);
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
	server_side = os_path_cat(name(), src->file_name);
	if (!is_update_prefix(server_side_0, server_side, opt.d))
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
	path = change_file_version_path(cp, src, &need_to_unlink);
	is_local = !!change_file_find(cp, src->file_name, view_path_first);
	os_become_orig();
	input ip = input_file_open(path, need_to_unlink);
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
	mode &= ~change_umask(cp);

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
		if (!minus_str)
		    minus_str = str_from_c("-");
		if (!zero)
		    zero = str_from_c("0");
		if
		(
		    str_equal(fip->version, minus_str)
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
		    new response_removed(client_side, server_side)
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
		new response_created
		(
		    client_side,
		    server_side,
		    ip,
		    mode,
		    version
		)
	    );
	}
	else if (opt.C)
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
		    new response_update_existing
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
		    new response_update_existing
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
		    new response_update_existing
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

	os_become_orig();
	ip.close();
	os_become_undo();
    }
    return true;
}


static bool
file_being_deleted(server_ty *sp, string_ty *server_side)
{
    file_info_ty    *fip;
    static string_ty *minus_str;

    fip = server_file_info_find(sp, server_side, 0);
    if (!fip)
	return false;
    if (!fip->version)
	return false;
    if (!minus_str)
	minus_str = str_from_c("-");
    return str_equal(fip->version, minus_str);
}


bool
module_change::checkin(server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    fstate_src_ty   *src;
    int             mode;
    string_ty       *version;
    string_ty       *filename;
    const char      *strp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    if (!cp->is_being_developed())
    {
	server_error
	(
	    sp,
	    "ci: project \"%s\": change %ld: this change must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(pp)->str_text,
	    cp->number
	);
	return false;
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

    src = change_file_find(cp, filename, view_path_first);
    if (!src)
    {
	if (file_being_deleted(sp, server_side))
	{
	    string_ty       *qp;
	    string_ty       *qf;
	    string_ty       *the_command;
	    int             ok;

	    qp = str_quote_shell(project_name_get(pp));
	    qf = str_quote_shell(filename);
	    the_command =
		str_format
		(
	  "aegis --remove-file --project=%s --change=%ld --base-relative -v %s",
		    qp->str_text,
		    cp->number,
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
		return false;

	    //
            // Change objects can be very long lived in the aecvsserver,
            // so make sure that we re-read the meta data soon.
            //
            change_lock_sync_forced(cp);

	    //
	    // Let the client know the file is well and truly gone.
            // (We dont use the Removed response because the file is
            // already deleted from thwe client.)
	    //
	    server_response_queue
	    (
		sp,
		new response_remove_entry(client_side, server_side)
	    );

	    //
	    // Report success.
	    //
	    return true;
	}

	server_error
	(
	    sp,
	    "ci: project \"%s\": change %ld: file \"%s\" unknown",
	    project_name_get(pp)->str_text,
	    cp->number,
	    filename->str_text
	);
	return false;
    }

    //
    // For changes which are being developeed, like this one, we
    // have to look at the file itself for the executable bit.
    //
    if (src->action != file_action_remove)
    {
	string_ty       *path;

	path = change_file_path(cp, filename);
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
    mode &= ~change_umask(cp);

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
	new response_checked_in(client_side, server_side, mode, version)
    );
    str_free(version);

    return true;
}


bool
module_change::add(server_ty *sp, string_ty *client_side,
    string_ty *server_side, const options &)
{
    fstate_src_ty   *src;
    int             mode;
    string_ty       *version;
    string_ty       *filename;
    const char      *strp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    if (!cp->is_being_developed())
    {
	server_error
	(
	    sp,
	    "add: project \"%s\": change %ld: this change must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(pp)->str_text,
	    cp->number
	);
	return false;
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
    src = change_file_find(cp, filename, view_path_first);
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

	    qp = str_quote_shell(project_name_get(pp));
	    qf = str_quote_shell(filename);
	    the_command =
		str_format
		(
		    "aegis -rmu -project=%s -change=%ld -base-relative -v %s",
		    qp->str_text,
		    cp->number,
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
		return false;
	    }

	    //
	    // Change objects can be very long lived in the aecvsserver,
	    // so make sure that we re-read the meta data soon.
	    //
	    change_lock_sync_forced(cp);

	    //
	    // Now re-get the file information.
	    //
	    src = project_file_find(pp, filename, view_path_extreme);
	    if (src)
	    {
		the_command =
		    str_format
		    (
		"aegis -copy-file -project=%s -change=%ld -base-relative -v %s",
			qp->str_text,
			cp->number,
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
		    return false;
		}

		//
		// Change objects can be very long lived in the aecvsserver,
		// so make sure that we re-read the meta data soon.
		//
		change_lock_sync_forced(cp);

		//
		// Now re-get the file information.
		//
		src = change_file_find(cp, filename, view_path_first);
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

	path = change_file_path(cp, filename);
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
    mode &= ~change_umask(cp);

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
	    new response_new_entry(client_side, server_side, mode, version)
	);
	str_free(version);

	return true;
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
	new response_new_entry(client_side, server_side, mode, version)
    );
    str_free(version);

    return true;
}


bool
module_change::remove(server_ty *sp, string_ty *client_side,
    string_ty *server_side, const options &)
{
    fstate_src_ty   *src;
    int             mode;
    string_ty       *version;
    string_ty       *filename;
    const char      *strp;

    //
    // It is an error if the change is not in the "being developed" state.
    //
    if (!cp->is_being_developed())
    {
	server_error
	(
	    sp,
	    "remove: project \"%s\": change %ld: this change must be "
		"in the \"being_developed\" state for this to work",
	    project_name_get(pp)->str_text,
	    cp->number
	);
	return false;
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
    src = change_file_find(cp, filename, view_path_first);
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

	qp = str_quote_shell(project_name_get(pp));
	qf = str_quote_shell(filename);
	the_command =
	    str_format
	    (
		"aegis %s -project=%s -change=%ld -base-relative -v %s",
		verb,
		qp->str_text,
		cp->number,
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
	    return false;

	//
	// Change objects can be very long lived in the aecvsserver,
	// so make sure that we re-read the meta data soon.
	//
	change_lock_sync_forced(cp);

	switch (src->action)
	{
	case file_action_create:
	    server_response_queue
	    (
		sp,
		new response_remove_entry(client_side, server_side)
	    );
	    return true;

	case file_action_remove:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
	src = 0;
    }

    src = project_file_find(pp, filename, view_path_extreme);
    if (!src)
    {
	server_error
	(
	    sp,
	    "project \"%s\": change %ld: file \"%s\" cannot be removed "
		"because it does not exist in the project",
	    project_name_get(pp)->str_text,
	    cp->number,
	    filename->str_text
	);
	return false;
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
	new response_new_entry(client_side, server_side, mode, version)
    );
    str_free(version);

    return true;
}


module_ty *
module_change_new(string_ty *project_name, long change_number)
{
    //
    // Make sure the project makes sense.
    //
    project_ty *pp = project_alloc(project_name);
    if (!pp->bind_existing_errok())
    {
	project_free(pp);
	return new module_project_bogus(project_name);
    }

    //
    // Make sure the change makes sense.
    //
    change::pointer cp = change_alloc(pp, change_number);
    if (!change_bind_existing_errok(cp))
    {
	change_free(cp);
	project_free(pp);
	return new module_change_bogus(project_name, change_number);
    }

    //
    // OK, looks like we have a viable change.
    //
    return new module_change(cp);
}
