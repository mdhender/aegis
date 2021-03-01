/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000-2002 Peter Miller;
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
 * MANIFEST: functions to manipulate verifys
 */

#include <cattr.h>
#include <change/attributes.h>
#include <sub.h>


void
change_attributes_verify(string_ty *fn, cattr d)
{
    if (!d->brief_description)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", fn);
	sub_var_set_charstar(scp, "FieLD_Name", "brief_description");
	fatal_intl(scp, i18n("$filename: contains no \"$field_name\" field"));
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!(d->mask & cattr_cause_mask))
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", fn);
	sub_var_set_charstar(scp, "FieLD_Name", "cause");
	fatal_intl(scp, i18n("$filename: contains no \"$field_name\" field"));
	/* NOTREACHED */
	sub_context_delete(scp);
    }
}
