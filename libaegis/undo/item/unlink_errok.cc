//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: implementation of the undo_item_unlink_errok class
//

#pragma implementation "undo_item_unlink_errok"

#include <os.h>
#include <sub.h>
#include <undo/item/unlink_errok.h>


undo_item_unlink_errok::~undo_item_unlink_errok()
{
}


undo_item_unlink_errok::undo_item_unlink_errok(const nstring &arg) :
    path(arg)
{
}


void
undo_item_unlink_errok::action()
{
    os_unlink_errok(path);
}


void
undo_item_unlink_errok::unfinished()
{
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", path);
    error_intl(scp, i18n("unfinished: rm $filename"));
    sub_context_delete(scp);
}
