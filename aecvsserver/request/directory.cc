//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
// //
// Directory <local-directory>
//
// Additional data: <repository>
//
// Response expected: no.
// Root required: yes.
//
// Tell the server what directory to use.  The <repository> should be
// a directory name from a previous server response.  Note that this
// both gives a default for 'Entry' and 'Modified' and also for 'ci'
// and the other commands; normal usage is to send 'Directory' for each
// directory in which there will be an 'Entry' or 'Modified', and then
// a final 'Directory' for the original directory, then the command.
// The <local-directory> is relative to the top level at which the
// command is occurring (i.e. the last 'Directory' which is sent before
// the command); to indicate that top level, "." should be sent for
// <local-directory>.
//
// Here is an example of where a client gets <repository> and
// <local-directory>.  Suppose that there is a module defined by
//
//      moddir 1dir
//
// That is, one can check out 'moddir' and it will take '1dir' in
// the repository and check it out to 'moddir' in the working
// directory.  Then an initial check out could proceed like this:
//
//      C: Root /home/kingdon/zwork/cvsroot
//      . . .
//      C: Argument moddir
//      C: Directory .
//      C: /home/kingdon/zwork/cvsroot
//      C: co
//      S: Clear-sticky moddir/
//      S: /home/kingdon/zwork/cvsroot/1dir/
//      . . .
//      S: ok
//
// In this example the response shown is 'Clear-sticky', but it could
// be another response instead.  Note that it returns two pathnames.
// The first one, "moddir/", indicates the working
// directory to check out into.  The second one, ending in "1dir/",
// indicates the directory to pass back to the server in a subsequent
// 'Directory' request.  For example, a subsequent 'update'
// request might look like:
//
//      C: Directory moddir
//      C: /home/kingdon/zwork/cvsroot/1dir
//      . . .
//      C: update
//
// For a given <local-directory>, the repository will be the same for
// each of the responses, so one can use the repository from whichever
// response is most convenient.  Typically a client will store the
// repository along with the sources for each <local-directory>, use
// that same setting whenever operating on that <local-directory>,
// and not update the setting as long as the <local-directory> exists.
//
// A client is free to rename a <local-directory> at any time (for
// example, in response to an explicit user request).  While it is true
// that the server supplies a <local-directory> to the client, as noted
// above, this is only the default place to put the directory.  Of course,
// the various 'Directory' requests for a single command (for example,
// 'update' or 'ci' request) should name a particular directory with
// the same <local-directory>.
//
// Each 'Directory' request specifies a brand-new <local-directory>
// and <repository>; that is, <local-directory> and <repository> are
// never relative to paths specified in any previous 'Directory' request.
//
// Here's a more complex example, in which we request an update of a
// working directory which has been checked out from multiple places in
// the repository.
//
//      C: Argument dir1
//      C: Directory dir1
//      C: /home/foo/repos/mod1
//      . . .
//      C: Argument dir2
//      C: Directory dir2
//      C: /home/foo/repos/mod2
//      . . .
//      C: Argument dir3
//      C: Directory dir3/subdir3
//      C: /home/foo/repos/mod3
//      . . .
//      C: update
//
// While directories 'dir1' and 'dir2' will be handled in similar fashion
// to the other examples given above, 'dir3' is slightly different from
// the server's standpoint.  Notice that module 'mod3' is actually
// checked out into 'dir3/subdir3', meaning that directory 'dir3' is
// either empty or does not contain data checked out from this repository.
//
// The above example will work correctly in cvs 1.10.1 and later.
// The server will descend the tree starting from all directories
// mentioned in 'Argument' requests and update those directories
// specifically mentioned in 'Directory' requests.
//
// Previous versions of cvs (1.10 and earlier) do not behave the same way.
// While the descent of the tree begins at all directories mentioned in
// 'Argument' requests, descent into subdirectories only occurs if a
// directory has been mentioned in a 'Directory' request.  Therefore, the
// above example would succeed in updating 'dir1' and 'dir2', but would
// skip 'dir3' because that directory was not specifically mentioned in
// a 'Directory' request.  A functional version of the above that would
// run on a 1.10 or earlier server is as follows:
//
//      C: Argument dir1
//      C: Directory dir1
//      C: /home/foo/repos/mod1
//      . . .
//      C: Argument dir2
//      C: Directory dir2
//      C: /home/foo/repos/mod2
//      . . .
//      C: Argument dir3
//      C: Directory dir3
//      C: /home/foo/repos/.
//      . . .
//      C: Directory dir3/subdir3
//      C: /home/foo/repos/mod3
//      . . .
//      C: update
//
// Note the extra 'Directory dir3' request.  It might be better to use
// 'Emptydir' as the repository for the 'dir3' directory, but the above
// will certainly work.
//
// One more peculiarity of the 1.10 and earlier protocol is the ordering
// of 'Directory' arguments.  In order for a subdirectory to be registered
// correctly for descent by the recursion processor, its parent must
// be sent first.  For example, the following would not work to update
// 'dir3/subdir3':
//
//      . . .
//      C: Argument dir3
//      C: Directory dir3/subdir3
//      C: /home/foo/repos/mod3
//      . . .
//      C: Directory dir3
//      C: /home/foo/repos/.
//      . . .
//      C: update
//
// The implementation of the server in 1.10 and earlier writes the
// administration files for a given directory at the time of the
// 'Directory' request.  It also tries to register the directory
// with its parent to mark it for recursion.  In the above example,
// at the time 'dir3/subdir3' is created, the physical directory for
// 'dir3' will be created on disk, but the administration files will
// not have been created.  Therefore, when the server tries to register
// 'dir3/subdir3' for recursion, the operation will silently fail because
// the administration files do not yet exist for 'dir3'.
//

#include <common/ac/assert.h>
#include <common/ac/string.h>

#include <aecvsserver/net.h>
#include <aecvsserver/request/directory.h>
#include <aecvsserver/server.h>


request_directory::~request_directory()
{
}


request_directory::request_directory()
{
}


void
request_directory::run_inner(server_ty *sp, string_ty *client_side)
    const
{
    static const char root_path[] = ROOT_PATH;
    size_t          root_path_len;

    assert(sp);
    assert(sp->np);
    if (server_root_required(sp, "Directory"))
        return;

    nstring server_side;
    if (!sp->np->getline(server_side))
    {
        server_error(sp, "Directory: additional data required");
        return;
    }

    //
    // Make sure the server side is below the root.
    // (Seems weird that the protocol doesn't elide this already.)
    //
    root_path_len = strlen(root_path);
    if
    (
        server_side.size() == root_path_len
    &&
        0 == memcmp(server_side.c_str(), root_path, root_path_len)
    )
    {
        server_side = ".";
    }
    else if
    (
        server_side.size() > root_path_len
    &&
        0 == memcmp(server_side.c_str(), root_path, root_path_len)
    &&
        server_side[root_path_len] == '/'
    )
    {
        //
        // Strip out the Root part, we don't need it because it's fake,
        // and it makes some of the other processing cumbersome.
        //
        server_side =
            nstring
            (
                server_side.c_str() + (root_path_len + 1),
                server_side.size()  - (root_path_len + 1)
            );
    }
    else
    {
        server_error(sp, "Directory: server-side path must include Root");
        return;
    }

    sp->np->directory_set(client_side, server_side.get_ref());
}


const char *
request_directory::name()
    const
{
    return "Directory";
}


bool
request_directory::reset()
    const
{
    return false;
}


// vim: set ts=8 sw=4 et :
