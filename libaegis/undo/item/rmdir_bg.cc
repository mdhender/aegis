//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/undo/item/rmdir_bg.h>


undo_item_rmdir_bg::~undo_item_rmdir_bg()
{
}


undo_item_rmdir_bg::undo_item_rmdir_bg(const nstring &arg) :
    path(arg)
{
}


void
undo_item_rmdir_bg::action()
{
    os_rmdir_bg(path);
}


void
undo_item_rmdir_bg::unfinished()
{
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", path);
    error_intl(scp, i18n("unfinished: rmdir $filename"));
    sub_context_delete(scp);
}
