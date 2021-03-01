//
//	aegis - project change supervisor
//	Copyright (C) 2005 Peter Miller;
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
// MANIFEST: implementation of the dir_functor_rm_dir_tree class
//

#pragma implementation "dir_functor_rm_dir_tree"

#include <commit.h>
#include <dir/functor/rm_dir_tree.h>
#include <nstring.h>
#include <os.h>
#include <trace.h>


dir_functor_rm_dir_tree::~dir_functor_rm_dir_tree()
{
}


dir_functor_rm_dir_tree::dir_functor_rm_dir_tree() :
    dir_functor()
{
}


void
dir_functor_rm_dir_tree::dir_functor_rm_dir_tree::operator()(msg_t msg,
    const nstring &path, const struct stat &st)
{
    trace(("rmdir_tree_callback(msg = %d, path = \"%s\", st = %08lX)\n{\n",
	msg, path.c_str(), (long)&st));
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
