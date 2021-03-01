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
#include <libaegis/sub.h>


void
change_warning_obsolete_field(change::pointer cp, string_ty *errpos,
    const char *old_field, const char *new_field)
{
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", errpos);
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
