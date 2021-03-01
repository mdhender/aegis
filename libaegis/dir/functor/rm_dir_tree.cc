//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <libaegis/commit.h>
#include <libaegis/dir/functor/rm_dir_tree.h>
#include <common/nstring.h>
#include <libaegis/os.h>
#include <common/trace.h>


dir_functor_rm_dir_tree::~dir_functor_rm_dir_tree()
{
}


dir_functor_rm_dir_tree::dir_functor_rm_dir_tree() :
    dir_functor()
{
}


void
dir_functor_rm_dir_tree::dir_functor_rm_dir_tree::operator()(msg_t msg,
    const nstring &path, const struct stat &)
{
    trace(("rmdir_tree_callback(msg = %d, path = \"%s\")\n{\n",
	msg, path.c_str()));
    trace_nstring(path);
    switch (msg)
    {
    case msg_dir_before:
	commit_rmdir_errok(path);
	os_chmod_errok(path, 0750);
	break;

    case msg_dir_after:
	break;

    case msg_file:
    case msg_special:
    case msg_symlink:
	commit_unlink_errok(path);
	break;
    }
    trace(("}\n"));
}
