//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate fatal_errors
//

#include <libaegis/input.h>
#include <libaegis/sub.h>


void
input_ty::fatal_error(const char *msg)
{
    sub_context_ty sc;
    string_ty *s = sc.subst_intl(msg);
    sc.var_set_string("File_Name", name());
    sc.var_set_string("MeSsaGe", s);
    sc.fatal_intl(i18n("$filename: $message"));
    // NOTREACHED
}
