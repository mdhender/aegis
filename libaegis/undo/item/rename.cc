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
// MANIFEST: implementation of the undo_item_rename class
//

#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/undo/item/rename.h>


undo_item_rename::~undo_item_rename()
{
}


undo_item_rename::undo_item_rename(const nstring &arg1, const nstring &arg2) :
    oldpath(arg1),
    newpath(arg2)
{
}


void
undo_item_rename::action()
{
    os_rename(oldpath, newpath);
}


void
undo_item_rename::unfinished()
{
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name1", oldpath);
    sub_var_set_string(scp, "File_Name2", newpath);
    error_intl(scp, i18n("unfinished: mv $filename1 $filename2"));
    sub_context_delete(scp);
}