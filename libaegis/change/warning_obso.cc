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
// MANIFEST: implementation of the change_warning_obso class
//

#include <change.h>
#include <sub.h>


void
change_warning_obsolete_field(change_ty *cp, string_ty *errpos,
    const char *old_field, const char *new_field)
{
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "FileName", errpos);
    sub_var_set_charstar(scp, "Name1", old_field);
    sub_var_set_charstar(scp, "Name2", new_field);
    change_warning
    (
	cp,
	scp,
	i18n("$filename: field \"$name1\" obsolete, use \"$name2\" instead")
    );
    sub_context_delete(scp);
}
