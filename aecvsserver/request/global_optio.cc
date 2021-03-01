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

#include <common/ac/string.h>

#include <aecvsserver/request/global_optio.h>
#include <aecvsserver/server.h>


request_global_option::~request_global_option()
{
}


request_global_option::request_global_option()
{
}


void
request_global_option::run_inner(server_ty *sp, string_ty *)
    const
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


const char *
request_global_option::name()
    const
{
    return "Global_option";
}


bool
request_global_option::reset()
    const
{
    return false;
}
