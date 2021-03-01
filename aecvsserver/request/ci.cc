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
// ci
//
// Actually do a CVS ci command.  This uses any previous Argument,
// Directory, Entry, or Modified requests, if they have been sent.
// The last Directory sent specifies the working directory at the time
// of the operation.  No provision is made for any input from the user.
// This means that ci must use a -m argument if it wants to specify a
// log message.
//
// Response expected: yes.
// Root required: yes.
//
//
// Example
//
// After the user modifies the file and instructs the client to check
// it back in.  The client sends arguments to specify the log message
// and file to check in:
//
//     C: Argument -m
//     C: Argument Well, you see, it took me hours and hours to find
//     C: Argumentx this typo and I searched and searched and eventually
//     C: Argumentx had to ask John for help.
//     C: Argument mungeall.c
//
// It also sends information about the contents of the working directory,
// including the new contents of the modified file.  Note that the user
// has changed into the "supermunger" directory before executing this
// command; the top level directory is a user-visible concept because
// the server should print filenames in M and E responses relative to
// that directory.
//
// (We are waving our hands about the order of the requests.  "Directory"
// and "Argument" can be in any order, but this probably isn't specified
// very well.)
//
//     C: Directory .
//     C: /u/cvsroot/supermunger
//     C: Entry /mungeall.c/1.1///
//     C: Modified mungeall.c
//     C: u=rw,g=r,o=r
//     C: 26
//     C: int main () { abort (); }
//
// And finally, the client issues the checkin command (which makes use
// of the data just sent):
//
//     C: ci
//
// And the server tells the client that the checkin succeeded:
//
//     S: M Checking in mungeall.c;
//     S: E /u/cvsroot/supermunger/mungeall.c,v  <--  mungeall.c
//     S: E new revision: 1.2; previous revision: 1.1
//     S: E done
//     S: Mode u=rw,g=r,o=r
//     S: Checked-in ./
//     S: /u/cvsroot/supermunger/mungeall.c
//     S: /mungeall.c/1.2///
//     S: ok
//
//
// Reverse Engineering Notes:
//
//     First comes the command line arguments from the client.
//     The following options are known to be transmitted by the client:
//         -m <text>          As two Argument requests, plus optional Argumentx
//
//     Then follows a series of Directory/Entry/Modified requests for
//     all the files which have changed in some way.
//
//     Then comes a Directory request, specifying what the
//     remaining arguments are relative to.
//
//     Then a series of Argument requests, listing all of the files to be
//     committed.  Conceivably this list could differ from the implicit
//     list you can derived from the Modified requests, because the -f
//     ("force") flag on the client side can be used to commit files
//     which have not been modified.
//
//     Last, they send the "ci" request.
//
//     The ugly part is that they could send more than one change at
//     a time!  CVS is more than happy to commit more than one module
//     at once.
//

#include <common/ac/string.h>

#include <aecvsserver/module.h>
#include <libaegis/os.h>
#include <aecvsserver/request/ci.h>
#include <aecvsserver/server.h>


request_checkin::~request_checkin()
{
}


request_checkin::request_checkin()
{
}


void
request_checkin::run_inner(server_ty *sp, string_ty *)
    const
{
    size_t          j;
    directory_ty    *dp;
    int             ok;

    if (server_root_required(sp, "ci"))
	return;
    if (server_directory_required(sp, "ci"))
	return;

    //
    // Skip the options.
    //
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
	if (arg->str_text[1] == 'm')
	    ++j;
    }

    ok = 1;
    dp = sp->np->get_curdir();
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
	// Pass the checkin to the relevant module, one file at a time.
	//
	// The CVS client is able to commit files to more than one module
	// in a single command.  This means we have to lookup the module
	// for every file.
	//
	module mp = module::find_trim(server_side);
	if (!mp->checkin(sp, client_side, server_side))
	    ok = 0;
	str_free(client_side);
	str_free(server_side);
	if (!ok)
	    break;
    }

    if (ok)
    {
	server_m
        (
	    sp,
	    "ci: now you have to use the \"aegis -develop-end\" command to "
		"finish the job"
	);
	server_ok(sp);
    }
}


const char *
request_checkin::name()
    const
{
    return "ci";
}


bool
request_checkin::reset()
    const
{
    return true;
}
