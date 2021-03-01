//
//	aegis - project change supervisor
//	Copyright (C) 2004-2007 Peter Miller
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
// MANIFEST: implementation of the quit_action_log class
//

#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/quit/action/log.h>


quit_action_log::~quit_action_log()
{
}


quit_action_log::quit_action_log()
{
}


void
quit_action_log::operator()(int)
{
    //
    // Skip the close (and the wait) if we were interrupted.
    //
    if (!os_interrupt_has_occurred())
	log_close();
}
