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
// update
//
// Actually do a cvs update command.  This uses any previous Argument,
// Directory, Entry, or Modified requests, if they have been sent.
// The last Directory sent specifies the working directory at the time
// of the operation.
//
// The -I option is not used -- files which the client can decide whether
// to ignore are not mentioned and the client sends the Questionable
// request for others.
//
// Root required: yes.
// Directory required: yes.
// Response expected: yes.
//
//
// From cvs(1)...
//
// Bring your working directory up to date with changes from the
// repository.  Merges are performed automatically when possible; a
// warning is issued if manual resolution is required for conflicting
// changes.  (Does not change repository.)
//
// update [ -ACdflPpQqR ][ -d ][ -r tag | -D date ] files...
//
// After you've run checkout to create your private copy of source
// from the common repository, other developers will continue changing
// the central source.  From time to time, when it is convenient in your
// development process, you can use the update command from within your
// working directory to reconcile your work with any revisions applied
// to the source repository since your last checkout or update.
//
// update keeps you informed of its progress by printing a line for
// each file, prefaced with one of the characters U P A R M C ?
// to indicate the status of the file:
//
// U file
//     The file was brought up to date with respect to the
//     repository.  This is done for any file that exists in the
//     repository but not in your source, and for files that you
//     haven't changed but are not the most recent versions available
//     in the repository.
//
// P file
//     Like U, but the CVS server sends a patch instead of an
//     entire file.  This accomplishes the same thing as U using
//     less bandwidth.
//
// A file
//     The file has been added to your private copy of the sources,
//     and will be added to the source repository when you run cvs
//     commit on the file.  This is a reminder to you that the file
//     needs to be committed.
//
// R file
//     The file has been removed from your private copy of the
//     sources, and will be removed from the source repository when
//     you run cvs commit on the file.  This is a reminder to you
//     that the file needs to be committed.
//
// M file
//     The file is modified in your working directory.   The M can
//     indicate one of two states for a file you are working on:
//     either there were no modifications to the same file in the
//     repository, so that your file remains as you last saw it; or
//     there were modifications in the repository as well as in your
//     copy, but they were merged successfully, without conflict,
//     in your working directory.
//
// C file
//     A conflict was detected while trying to merge your changes to
//     file with changes from the source repository. file (the copy
//     in your working directory) is now the result of merging the
//     two versions; an unmodified copy of your file is also in your
//     working directory, with the name .#file.version, where
//     version is the revision that your modified file started from.
//     (Note that some systems automatically purge files that begin
//     with .# if they have not been accessed for a few days.
//     If you intend to keep a copy of your original file, it is a
//     very good idea to rename it.)
//
// ? file
//     file is in your working directory, but does not correspond to
//     anything in the source repository, and is not in the list of
//     files for cvs to ignore (see the description of the -I option).
//
// Use the -A option to reset any sticky tags, dates, or -k options.
// (If you get a working copy of a file by using one of the -r, -D,
// or -k options, cvs remembers the corresponding tag, date, or kflag
// and continues using it on future updates; use the -A option to
// make cvs forget these specifications, and retrieve the head
// version of the file).
//
// The -jbranch option merges the changes made between the resulting
// revision and the revision that it is based on (e.g., if the tag
// refers to a branch, cvs will merge all changes made in that branch
// into your working file).
//
// With two -j options, cvs will merge in the changes between the two
// respective revisions.  This can be used to remove a certain
// delta from your working file.  E.g., If the file foo.c is based on
// revision 1.6 and I want to remove the changes made between 1.3 and
// 1.5, I might do:
//
//     example% cvs update -j1.5 -j1.3 foo.c # note the order...
//
// In addition, each -j option can contain on optional date specification
// which, when used with branches, can limit the chosen revision to one
// within a specific date.  An optional date is specified by adding a
// colon (:) to the tag.
//
//     -jSymbolic_Tag:Date_Specifier
//
// Use the -d option to create any directories that exist in the
// repository if they're missing from the working directory. (Normally,
// update acts only on directories and files that were already enrolled
// in your working directory.)  This is useful for updating directories
// that were created in the repository since the initial checkout; but it
// has an unfortunate side effect.  If you deliberately avoided certain
// directories in the repository when you created your working directory
// (either through use of a module name or by listing explicitly the
// files and directories you wanted on the command line), then updating
// with -d will create those directories, which may not be what you want.
//
// Use -I name to ignore files whose names match name (in your working
// directory) during the update.  You can specify -I more than once on
// the command line to specify several files to ignore.  By default,
// update ignores files whose names match certain patterns; for an up
// to date list of ignored file names, see the Cederqvist manual (as
// described in the SEE ALSO section of this manpage).
//
// Use -I ! to avoid ignoring any files at all.
//
// Use the -C option to overwrite locally modified files with
// clean copies from the repository (the modified file is saved in
// .#file.revision, however).
//
// The standard cvs command options -f, -k, -l, -P, -p, and -r are
// also available with update.
//
//
// From src/update.c
//   -A       Reset any sticky tags/date/kopts.
//   -P       Prune empty directories.
//   -C       Overwrite locally modified files with clean repository copies.
//   -d       Build directories, like checkout does.
//   -f       Force a head revision match if tag/date not found.
//   -l       Local directory only, no recursion.
//   -R       Process directories recursively.
//   -p       Send updates to standard output (avoids stickiness).
//   -k kopt  Use RCS kopt -k option on checkout. (is sticky)
//   -r rev   Update using specified revision/tag (is sticky).
//   -D date  Set date to update from (is sticky).
//   -j rev   Merge in changes made between current revision and rev.
//   -I ign   More files to ignore (! to reset).
//   -W spec  Wrappers specification line.
//
//
// Reverse Engineering Notes:
//
// 1.  You can tell that files exist on the client side because the
//     client sent Entry or Modified or Questionable or Unchanged requests
//     for each of them.
//
// 2.  You can tell if directories exist on the client side because
//     the client sent Directory requests for each of them.
//
// 3.  Directories must exist before you create new files in them, or
//     create new directories in them, otherwise the client will give
//     the user an error.  Create directories using the Clear-sticky
//     and Clear-static-directory responses.
//
// 4.  You must create new files using the Created response.  Do not send
//     this for existing client-side files, or the client will give the
//     user an error.
//
// 5.  You must update existing files using the Update-existing or Merged
//     responses.  Do not send these responses for files which do not exist
//     on the client-side, or the client will give the user an error.
//
// 6.  The client can update more than one module in the one request.
//     This is because the client recursively walks the client side
//     directory tree without regard for modules.  Modules would appear
//     to be simple directory name aliases.
//

#include <common/ac/string.h>

#include <aecvsserver/module.h>
#include <libaegis/os.h>
#include <aecvsserver/request/update.h>
#include <aecvsserver/server.h>


request_update::~request_update()
{
}


request_update::request_update()
{
}


void
request_update::run_inner(server_ty *sp, string_ty *)
    const
{
    size_t          j;

    if (server_root_required(sp, "update"))
	return;
    if (server_directory_required(sp, "update"))
	return;

    //
    // Process the options.
    //
    module_ty::options opt;
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
	case 'A':
	    //
	    // Reset any sticky tags/date/kopts.
	    //
	    opt.A++;
	    break;

	case 'C':
	    //
	    // Overwrite locally modified files with clean repository
	    // copies.
	    //
	    opt.C++;
	    break;

	case 'D':
	    //
	    // Set date to update from (is sticky).
	    //
	    opt.D = str_copy(sp->np->argument_nth(++j));
	    break;

	case 'd':
	    //
	    // Build directories, like checkout does.
	    //
	    opt.d++;
	    break;

	case 'f':
	    //
	    // Force a head revision match if tag/date not found.
	    //
	    opt.f++;
	    break;

	case 'I':
	    //
	    // More files to ignore (! to reset).
	    //
	    opt.I = str_copy(sp->np->argument_nth(++j));
	    break;

	case 'j':
	    //
	    // Merge in changes made between current revision and rev.
	    //
	    opt.j = str_copy(sp->np->argument_nth(++j));
	    break;

	case 'k':
	    //
	    // Use RCS kopt -k option on checkout. (is sticky)
	    //
	    opt.k = str_copy(sp->np->argument_nth(++j));
	    break;

	case 'l':
	    //
	    // Local directory only, no recursion.
	    //
	    opt.l++;
	    break;

	case 'P':
	    //
	    // Prune empty directories.
	    //
	    opt.P++;
	    break;

	case 'p':
	    //
	    // Send updates to standard output (avoids stickiness).
	    //
	    opt.p++;
	    break;

	case 'R':
	    //
	    // Process directories recursively.
	    //
	    opt.R++;
	    break;

	case 'r':
	    //
	    // Update using specified revision/tag (is sticky).
	    //
	    opt.r = str_copy(sp->np->argument_nth(++j));
	    break;

	case 'W':
	    //
	    // Wrappers specification line.
	    //
	    opt.W = str_copy(sp->np->argument_nth(++j));
	    break;

	default:
	    server_e(sp, "update: unknown '%s' option", arg->str_text);
	    break;
	}
    }

    //
    // Now process the rest of the arguments.
    // Each is a file or directory to be updated.
    //
    bool ok = true;
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
	// Pass the update to the relevant module, one argument at a time.
	//
	// The CVS client is able to update files in more than one module
	// in a single command.  This means we have to lookup the module
	// for every argument.
	//
	module mp = module::find_trim(server_side);
	if (!mp->update(sp, client_side, server_side, opt))
	    ok = false;
	str_free(client_side);
	str_free(server_side);

	//
	// Bail if something went wrong.
	//
	if (!ok)
	    break;
    }

    if (ok)
	server_ok(sp);
}


const char *
request_update::name()
    const
{
    return "update";
}


bool
request_update::reset()
    const
{
    return true;
}
