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
// remove
//
// Remove a file.  This uses any previous Argument, Directory, Entry,
// or Modified requests, if they have been sent.  The last Directory
// sent specifies the working directory at the time of the operation.
//
// Root required: yes.
// Response expected: yes.
//
// Note that this request does not actually do anything to the repository;
// the only effect of a successful "remove" request is to supply the
// client with a new entries line containing /-/ to indicate a removed
// file.  In fact, the client probably could perform this operation
// without contacting the server, although using "remove" may cause the
// server to perform a few more checks.
//
// The client sends a subsequent "ci" request to actually record the
// removal in the repository.
//
//
// From the cvs(1) man page:
//
// cvs remove file...
//     Use this command (after erasing any files listed) to declare that
//     you wish to eliminate files from the repository.  The removal
//     does not affect others until you run cvs commit.
//
// remove
//     Remove files from the source repository, pending a cvs commit on
//     the same files.  (Does not directly affect repository; changes
//     working directory.)
//
// remove [-lR] [files...]
//     Requires: Working directory.
//     Changes: Working directory.
//     Synonyms: rm, delete
//     Use this command to declare that you wish to remove files from the
//     source repository.  Like most cvs commands, cvs remove works on
//     files in your working directory, not directly on the repository.
//     As a safeguard, it also requires that you first erase the specified
//     files from your working directory.
//
//     The files are not actually removed until you apply your changes to
//     the repository with commit; at that point, the corresponding RCS
//     files in the source repository are moved into the Attic directory
//     (also within the source repository).
//
//     This command is recursive by default, scheduling all physically
//     removed files that it finds for removal by the next commit.
//     Use the -l option to avoid this recursion, or just specify that
//     actual files that you wish remove to consider.
//
//
// Reverse Engineering Notes:
//

#include <common/ac/string.h>

#include <common/error.h> // HACK
#include <aecvsserver/module.h>
#include <libaegis/os.h>
#include <aecvsserver/request/remove.h>
#include <aecvsserver/server.h>


request_remove::~request_remove()
{
}


request_remove::request_remove()
{
}


void
request_remove::run_inner(server_ty *sp, string_ty *)
    const
{
    size_t          j;

    if (server_root_required(sp, "remove"))
	return;
    if (server_directory_required(sp, "remove"))
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
	case 'f':
	    //
            // Force a head revision match if tag/date not found.
            //
	    opt.f++;
	    break;

	case 'l':
	    //
            // Local directory only, no recursion.
            //
	    opt.l++;
	    break;

	case 'R':
	    //
            // Process directories recursively.
            //
	    opt.R++;
	    break;

	default:
	    server_e(sp, "remove: unknown '%s' option", arg->str_text);
	    break;
	}
    }

    //
    // Now process the rest of the arguments.
    // Each is a file or directory to be added.
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
	// If they have asked for a directory to be removed, we politely
	// ignore the request, because Aegis doesn't track directory
	// existence, except as implied by file existence.
	//
	if (sp->np->directory_find_client_side(client_side))
	{
	    server_m
	    (
		sp,
		"Directory " ROOT_PATH "/%s removed from the repository "
		    "(ignored)",
		server_side->str_text
	    );
	    str_free(client_side);
	    str_free(server_side);
	    continue;
	}

	//
	// Pass the remove to the relevant module, one argument at a time.
	//
	// The CVS client is able to remove files in more than one module
	// in a single command.  This means we have to lookup the module
	// for every argument.
	//
	module mp = module::find_trim(server_side);
	if (!mp->remove(sp, client_side, server_side, opt))
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
    {
	server_e(sp, "use 'cvs commit' to make the 'cvs remove' permanent");
	server_ok(sp);
    }
}


const char *
request_remove::name()
    const
{
    return "remove";
}


bool
request_remove::reset()
    const
{
    return true;
}
