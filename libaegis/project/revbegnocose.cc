//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate revbegnocoses
//

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/project/history.h>


void
project_review_begin_notify_command_set(project_ty *pp, string_ty *value)
{
    change_ty       *cp;

    cp = pp->change_get();
    change_branch_review_begin_notify_command_set(cp, value);
}
