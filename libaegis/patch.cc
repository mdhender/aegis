//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2006, 2008 Peter Miller
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

#include <libaegis/patch.h>


patch_ty::patch_ty()
{
    action = (file_action_ty)-1;
    usage = (file_usage_ty)-1;
    patch_hunk_list_constructor(&actions);
}


patch_ty::~patch_ty()
{
    patch_hunk_list_destructor(&actions);
}


void
patch_ty::append(patch_hunk_ty *php)
{
    patch_hunk_list_append(&actions, php);
}
