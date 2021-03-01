//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller
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
// MANIFEST: implementation of the quit_action_undo class
//

#include <libaegis/os.h>
#include <libaegis/quit/action/undo.h>
#include <libaegis/undo.h>


quit_action_undo::~quit_action_undo()
{
}


quit_action_undo::quit_action_undo() :
    quit_action()
{
}


void
quit_action_undo::operator()(int exit_status)
{
    if (exit_status != 0)
    {
	os_become_undo_atexit();
	undo();
    }
}
