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

#include <libaegis/change.h>
#include <common/str_list.h>


bool
change_reviewer_already(change::pointer cp, const nstring &login)
{
    string_list_ty staff;
    change_reviewer_list(cp, staff);
    return staff.member(login.get_ref());
}


bool
change_reviewer_already(change::pointer cp, string_ty *login)
{
    return change_reviewer_already(cp, nstring(login));
}
