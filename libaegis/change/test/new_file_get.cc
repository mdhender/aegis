//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate new_file_gets
//

#include <change.h>
#include <error.h> // for assert
#include <sub.h>


string_ty *
change_new_test_filename_get(change_ty *cp, long n, int is_auto)
{
	sub_context_ty	*scp;
	pconf_ty        *pconf_data;
	string_ty	*result;

	assert(cp->reference_count >= 1);
	scp = sub_context_new();
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data->new_test_filename);
	sub_var_set_long(scp, "Hundred", n / 100);
	sub_var_optional(scp, "Hundred");
	sub_var_set_long(scp, "Number", n);
	// do not gettext these next two names
	sub_var_set_charstar(scp, "Type", (is_auto ? "automatic" : "manual"));
	sub_var_optional(scp, "Type");
	result = substitute(scp, cp, pconf_data->new_test_filename);
	sub_context_delete(scp);
	return result;
}
