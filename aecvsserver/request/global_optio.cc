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
// MANIFEST: functions to manipulate global_optios
//
//
// Global_option <option>
//
// Transmit one of the global options -q, -Q, -l, -t, -r, or -n.
// The <option> must be one of those strings, no variations (such
// as combining of options) are allowed.  For graceful handling of
// "valid-requests", it is probably better to make new global options
// separate requests, rather than trying to add them to this request.
//
// Response expected: no.
// Root required: no.
//

#include <ac/string.h>

#include <request/global_optio.h>
#include <server.h>


static void
run(server_ty *sp, string_ty *fn)
{
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
	//
        // From cvs(1) we glean the following options:
	//
	//  -l  Do not log the cvs_command in the command history (but
	//      execute it anyway).  See the description of the history
	//      command for information on command history.
	//
	//  -n  Do not change any files.  Attempt to execute the cvs_command,
	//      but only to issue reports; do not remove, update, or merge
	//      any existing files, or create any new files.
	//
	//  -Q  Causes the command to be really quiet; the command will
	//      generate output only for serious problems.
	//
	//  -q  Causes the command to be somewhat quiet; informational
	//      messages, such as	reports of recursion through
	//      subdirectories, are suppressed.
	//
	//  -r  Makes new working files read-only.  Same effect as if the
	//      CVSREAD environment variable is set.
	//
	//  -t  Trace program execution; display messages showing the steps
	//      of cvs activity.  Particularly useful with -n to explore
	//      the potential impact of an unfamiliar command.
	//
	// But there is no statement that's what the protocol actually wants.
	//
    }

    //
    // It would help of the documentation actually said what this
    // request does.  For now we ignore this request.
    //
}


const request_ty request_global_option =
{
    "Global_option",
    run,
    0, // no reset
};
