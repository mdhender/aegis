/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate fatal_errors
 */

#include <input.h>
#include <sub.h>


void
input_fatal_error(input_ty *fp, const char *msg)
{
	sub_context_ty	*scp;
	string_ty	*s;

	scp = sub_context_new();
	s = subst_intl(scp, msg);
	sub_var_set_string(scp, "File_Name", input_name(fp));
	sub_var_set_string(scp, "MeSsaGe", s);
	fatal_intl(scp, i18n("$filename: $message"));
	/* NOTREACHED */
}
