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
//
// add
//
// Add a file or directory.  This uses any previous Argument, Directory,
// Entry, or Modified requests, if they have been sent.  The last
// Directory sent specifies the working directory at the time of the
// operation.
//
// Root required: yes.
// Directory required: yes.
// Response expected: yes.
//
//
// Directory Example:
//
// To add a directory, send the directory to be added using Directory
// and Argument requests.
//
//     C: Root /u/cvsroot
//     . . .
//     C: Argument nsdir
//     C: Directory nsdir
//     C: /u/cvsroot/1dir/nsdir
//     C: Directory .
//     C: /u/cvsroot/1dir
//     C: add
//     S: M Directory /u/cvsroot/1dir/nsdir added to the repository
//     S: ok
//
// You will notice that the server does not signal to the client in any
// particular way that the directory has been successfully added.  The
// client is supposed to just assume that the directory has been added and
// update its records accordingly.  Note also that adding a directory is
// immediate; it does not wait until a "ci" request as files do.
//
//
// File Example:
//
// To add a file, send the file to be added using a Modified request.
//
//     C: Argument nfile
//     C: Directory .
//     C: /u/cvsroot/1dir
//     C: Is-modified nfile
//     C: add
//     S: E cvs server: scheduling file `nfile' for addition
//     S: Mode u=rw,g=r,o=r
//     S: Checked-in ./
//     S: /u/cvsroot/1dir/nfile
//     S: /nfile/0///
//     S: E cvs server: use 'cvs commit' to add this file permanently
//     S: ok
//
// Note that the file has not been added to the repository; the only
// effect of a successful add request, for a file, is to supply the
// client with a new entries line containing 0 to indicate an added file.
// In fact, the client probably could perform this operation without
// contacting the server, although using add does cause the server to
// perform a few more checks.
//
// The client sends a subsequent "ci" to actually add the file to the
// repository.
//
// Another quirk of the add request is that with CVS 1.9 and older,
// a pathname specified in an Argument request cannot contain "/".
// There is no good reason for this restriction, and in fact more
// recent CVS servers don't have it.  But the way to interoperate with
// the older servers is to ensure that all Directory requests for "add"
// (except those used to add directories, as described above), use "." for
// <local-directory>.  Specifying another string for <local-directory>
// may not get an error, but it will get you strange Checked-in responses
// from the buggy servers.
//
//
// From the cvs(1) man page:
//
// cvs add file...
//     Use this command to enroll new files in cvs records of your working
//     directory.  The files will be added to the repository the next time
//     you run cvs commit.  Note: You should use the cvs import command
//     to bootstrap new sources into the source repository.  The cvs add
//     comman is only used for adding new files to an already checked-out
//     module.
//
// add
//     Add a new file or directory to the repository, pending a cvs
//     commit on the same file.  Can only be done from within sources
//     created by a previous cvs checkout invocation.  Use cvs import
//     to place whole new hierarchies of sources under cvs control.
//     (Does not directly affect repository; changes working directory.)
//
// add [-k kflag] [-m message] files...
//     Use the add command to create a new file or directory in the
//     source repository.  The files or directories specified with add
//     must already exist in the current directory (which must have been
//     created with the checkout command).  To add a whole new directory
//     hierarchy to the source repository (for example, files received
//     from a third-party vendor), use the cvs import command instead.
//
//     If the argument to cvs add refers to an immediate sub-directory,
//     the directory is created at the correct place in the source
//     repository, and the necessary cvs administration files are created
//     in your working directory.  If the directory already exists in
//     the source repository, cvs add still creates the administration
//     files in your version of the directory.  This allows you to use
//     cvs add to add a particular directory to your private sources
//     even if someone else created that directory after your checkout
//     of the sources.  You can do the following:
//
//         example% mkdir new_directory
//         example% cvs add new_directory
//         example% cvs update new_directory
//
//     An alternate approach using cvs update might be:
//
//         example% cvs update -d new_directory
//
//     (To add any available new directories to your working directory,
//     it's probably simpler to use cvs checkout or cvs update -d.)
//
//     The added files are not placed in the source repository until
//     you use cvs commit to make the change permanent.  Doing a cvs
//     add on a file that was removed with the cvs remove command will
//     resurrect the file, if no cvs commit command intervened.
//
//     You will have the opportunity to specify a logging message, as
//     usual, when you use cvs commit to make the new file permanent.
//     If you'd like to have another logging message associated with just
//     creation of the file (for example, to describe the file's purpose),
//     you can specify it with the -m message option to the add command.
//
//     The -k kflag option specifies the default way that this file will
//     be checked out.  The kflag argument is stored in the RCS file
//     and can be changed with cvs admin.  Specifying -ko is useful for
//     checking in binaries that shouldn't have keywords expanded.
//
//
// The user files or directories must already exist (on the client side).
//
// An "add" on a file that has been "remove"d but not committed will
// cause the file to be resurrected.
//
//
// Reverse Engineering Notes:
//
//     The cvsclient.texi file has it wrong, it says that Modified is
//     sent, but only Is-modified is sent.
//

#include <common/ac/string.h>

#include <aecvsserver/module.h>
#include <libaegis/os.h>
#include <aecvsserver/request/add.h>
#include <aecvsserver/server.h>


request_add::~request_add()
{
}


request_add::request_add()
{
}


void
request_add::run_inner(server_ty *sp, string_ty *)
    const
{
    if (server_root_required(sp, "add"))
	return;
    if (server_directory_required(sp, "add"))
	return;

    //
    // Process the options.
    //
    module_ty::options opt;
    size_t j = 0;
    for (j = 0; j < sp->np->argument_count(); ++j)
    {
	string_ty *arg = sp->np->argument_nth(j);
	if (arg->str_text[0] != '-')
	    break;
	if (0 == strcmp(arg->str_text, "--"))
	{
	    ++j;
	    break;
	}
	switch (arg->str_text[1])
	{
	case 'k':
	    //
	    // Use RCS kopt -k option on checkout. (is sticky)
	    //
	    opt.k = str_copy(sp->np->argument_nth(++j));
	    break;

	case 'm':
	    //
	    // Logging message associated with the file.
	    //
	    opt.m = str_copy(sp->np->argument_nth(++j));
	    break;

	default:
	    server_e(sp, "add: unknown '%s' option", arg->str_text);
	    break;
	}
    }

    //
    // Now process the rest of the arguments.
    // Each is a file or directory to be added.
    //
    bool ok = 1;
    directory_ty *dp = sp->np->get_curdir();
    for (; j < sp->np->argument_count(); ++j)
    {
	string_ty       *arg;
	string_ty       *client_side;
	string_ty       *server_side;

	//
	// Build the (more complete) name of the file on both the client
	// side and the server side.
	//
	arg = sp->np->argument_nth(j);
	client_side = os_path_cat(dp->client_side, arg);
	server_side = os_path_cat(dp->server_side, arg);

	//
	// If they have asked for a directory to be added, we politely
	// ignore the request, because Aegis doesn't track directory
	// existence, except as implied by file existence.
	//
	if (sp->np->directory_find_client_side(client_side))
	{
	    server_m
	    (
		sp,
		"Directory " ROOT_PATH "/%s added to the repository (ignored)",
		server_side->str_text
	    );
	    str_free(client_side);
	    str_free(server_side);
	    continue;
	}

	//
	// Pass the add to the relevant module, one argument at a time.
	//
	// The CVS client is able to add files in more than one module
	// in a single command.  This means we have to lookup the module
	// for every argument.
	//
	module mp = module::find_trim(server_side);
	if (!mp->add(sp, client_side, server_side, opt))
	    ok = 0;
	str_free(client_side);
	str_free(server_side);

	//
	// Bail if something went wrong.
	//
	if (!ok)
	    break;
    }

    if (ok)
    {
	server_e(sp, "use 'cvs commit' to make the 'cvs add' permanent");
	server_ok(sp);
    }
}


const char *
request_add::name()
    const
{
    return "add";
}


bool
request_add::reset()
    const
{
    return true;
}
