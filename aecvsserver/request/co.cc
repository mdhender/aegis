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
// MANIFEST: functions to manipulate cos
//
//
// co
//
// Get files from the repository.  This uses any previous Argument,
// Directory, Entry, or Modified requests, if they have been sent.
// Arguments to this command are module names; the client cannot know
// what directories they correspond to except by (1) just sending the
// co request, and then seeing what directory names the server sends
// back in its responses, and (2) the expand-modules request.
//
// Response expected: yes.
// Root required: yes.
//
//
// Example:
//
// The client wants to check out the "supermunger" module into a fresh
// working directory.  Therefore it first expands the "supermunger"
// module; this step would be omitted if the client was operating on a
// directory rather than a module.
//
//	C: Argument supermunger
//	C: Directory .
//	C: /u/cvsroot
//	C: expand-modules
//
// The server replies that the "supermunger" module expands to the
// directory "supermunger" (the simplest case):
//
//	S: Module-expansion supermunger
//	S: ok
//
// The client then proceeds to check out the directory.  The fact that
// it sends only a single Directory request which specifies "."  for the
// working directory means that there is not already a "supermunger"
// directory on the client.
//
// (According to cvs(1) that -N means avoid shortening module paths
// in your working directory.  It appears to be a hack to get the CVS
// server-side to work; it would have been more appropriate if they
// had changed the server side rather than the client side.  We will
// ignore it.)
//
//	C: Argument -N
//	C: Argument supermunger
//	C: Directory .
//	C: /u/cvsroot
//	C: co
//
// The server replies with the requested files.  In this example,
// there is only one file, "mungeall.c".  The Clear-sticky and
// Clear-static-directory requests are sent by the current implementation
// but they have no effect because the default is for those settings to
// be clear when a directory is newly created.
//
//	S: Clear-sticky supermunger/
//	S: /u/cvsroot/supermunger/
//	S: Clear-static-directory supermunger/
//	S: /u/cvsroot/supermunger/
//	S: E cvs server: Updating supermunger
//	S: M U supermunger/mungeall.c
//	S: Created supermunger/
//	S: /u/cvsroot/supermunger/mungeall.c
//	S: /mungeall.c/1.1///
//	S: u=rw,g=r,o=r
//	S: 26
//	S: int mein () { abort (); }
//	S: ok
//

#include <ac/string.h>

#include <module.h>
#include <request/co.h>
#include <server.h>


static void
run(server_ty *sp, string_ty *arg)
{
    module_ty       *mp;
    size_t          j;

    if (server_root_required(sp, "co"))
	return;
    if (server_directory_required(sp, "co"))
	return;
    for (j = 0; j < sp->np->argument_list.nstrings; ++j)
    {
	if (sp->np->argument_list.string[j]->str_text[0] != '-')
	    break;
	//
	// St the moment, we ignore all options
	//
	// -A
	// -c
	//     Use the -c option to copy the module file, sorted, to the
	//     standard output, instead of creating or modifying any
	//     files or directories in your working directory.
	// -d dir
	//     Use this option to create a directory called dir for the
	//     working files, instead of using the module name.
	// -D date
	// -f
	// -j branch
	// -k flag
	// -l
	// -n
	// -N
	//     Use this to avoid shortening module paths in your working
	//     directory.
	// -p
	// -P
	// -r tag
	// -s
	//     Display per-module status information stored with the -s
	//     option within the modules file.
	//
    }
    if (j != sp->np->argument_list.nstrings - 1)
    {
	server_error(sp, "co: no module name specified");
	return;
    }

    mp = module_find(sp->np->argument_list.string[j]);
    module_checkout(mp, sp);
    module_delete(mp);
}


const request_ty request_co =
{
    "co",
    run,
    1, // reset
};
