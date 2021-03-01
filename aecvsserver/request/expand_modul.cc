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
// expand-modules
//
// Expand the modules which are specified in the arguments.  Returns the
// data in Module-expansion responses.  Note that the server can assume
// that this is checkout or export, not rtag or rdiff; the latter do not
// access the working directory and thus have no need to expand modules
// on the client side.
//
// Expand may not be the best word for what this request does.  It does
// not necessarily tell you all the files contained in a module,
// for example.  Basically it is a way of telling you which working
// directories the server needs to know about in order to handle a
// checkout of the specified modules.
//
// Directory expected: yes.
// Response expected: yes.
// Root required: yes.
//
//
// For example, suppose that the server has a module defined by
//
//	aliasmodule -a 1dir
//
// That is, one can check out aliasmodule and it will take 1dir in
// the repository and check it out to 1dir in the working directory.
// Now suppose the client already has this module checked out and
// is planning on using the co request to update it.  Without using
// expand-modules, the client would have two bad choices: it could
// either send information about *all* working directories under the
// current directory, which could be unnecessarily slow, or it could be
// ignorant of the fact that aliasmodule stands for 1dir, and neglect
// to send information for 1dir, which would lead to incorrect operation.
//
// With expand-modules, the client would first ask for the module to
// be expanded:
//
//	C: Root /home/kingdon/zwork/cvsroot
//	. . .
//	C: Argument aliasmodule
//	C: Directory .
//	C: /home/kingdon/zwork/cvsroot
//	C: expand-modules
//	S: Module-expansion 1dir
//	S: ok
//
// and then it knows to check the "1dir" directory and send requests
// such as Entry and Modified for the files in that directory.
//

#include <common/error.h> // for assert
#include <aecvsserver/module.h>
#include <aecvsserver/response/error.h>
#include <aecvsserver/response/module_expan.h>
#include <aecvsserver/request/expand_modul.h>
#include <aecvsserver/server.h>


request_expand_modules::~request_expand_modules()
{
}


request_expand_modules::request_expand_modules()
{
}


void
request_expand_modules::run_inner(server_ty *sp, string_ty *)
    const
{
    size_t         j;

    assert(sp);
    if (server_root_required(sp, "expand-modules"))
	return;
    if (server_directory_required(sp, "expand-modules"))
	return;

    //
    // There should be exactly one argument, and exactly one directory.
    // We actually ignore the directory.
    //
    if (sp->np->argument_count() < 1)
    {
	server_error(sp, "expand-modules: at least one Argument required");
	return;
    }
    for (j = 0; j < sp->np->argument_count(); ++j)
    {
	string_ty *aname = sp->np->argument_nth(j);
	module mp = module::find(aname);
	if (mp->is_bogus())
	{
	    response_error *rp =
		new response_error
		(
		    str_format
		    (
			"expand-modules: module \"%s\" unknown",
			aname->str_text
		    ),
		    str_from_c("ENOENT")
		);
	    server_response_queue(sp, rp);
	    return;
	}

	//
	// We don't use the name we were given, we ask the module what
	// its name is and return that.  This will translate Aegis'
	// project aliases.
	//
	server_response_queue
	(
	    sp,
	    new response_module_expansion(mp->name())
	);
    }
    server_ok(sp);
}


const char *
request_expand_modules::name()
    const
{
    return "expand-modules";
}


bool
request_expand_modules::reset()
    const
{
    return true;
}
