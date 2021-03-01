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
// MANIFEST: implementation of the undo_item class
//

#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/undo/item.h>
#include <common/trace.h>


undo_item::~undo_item()
{
}


undo_item::undo_item() :
    uid(0),
    gid(0),
    umask(022)
{
    os_become_query(&uid, &gid, &umask);
    assert(uid != 0);
    trace(("uid = %d\n", uid));
    // assert(gid != 0);
    trace(("gid = %d\n", gid));
}


void
undo_item::act()
{
    os_become(uid, gid, umask);
    action();
    os_become_undo();
}


void
undo_item::unfinished()
{
    // do nothing
}
