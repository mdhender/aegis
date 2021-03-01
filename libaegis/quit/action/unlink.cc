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
// MANIFEST: implementation of the quit_action_unlink class
//

#pragma implementation "quit_action_unlink"

#include <error.h> // for assert
#include <os.h>
#include <quit/action/unlink.h>


quit_action_unlink::~quit_action_unlink()
{
}


quit_action_unlink::quit_action_unlink(const nstring &arg) :
    path(arg),
    uid(0),
    gid(0),
    umask(0)
{
    os_become_query(&uid, &gid, &umask);
    assert(uid != 0);
    assert(gid != 0);
}


void
quit_action_unlink::operator()(int exit_status)
{
    // This action is NOT conditional on the exit status.
    os_become(uid, gid, umask);
    os_unlink_errok(path);
    os_become_undo();
}
