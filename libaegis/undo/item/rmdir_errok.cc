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
// MANIFEST: implementation of the undo_item_rmdir_errok class
//

#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/undo/item/rmdir_errok.h>


undo_item_rmdir_errok::~undo_item_rmdir_errok()
{
}


undo_item_rmdir_errok::undo_item_rmdir_errok(const nstring &arg) :
    path(arg)
{
}


void
undo_item_rmdir_errok::action()
{
    os_rmdir_errok(path);
}


void
undo_item_rmdir_errok::unfinished()
{
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", path);
    error_intl(scp, i18n("unfinished: rmdir $filename"));
    sub_context_delete(scp);
}
