/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate pronamtoolons
 */

#include <help.h>
#include <sub.h>


void
fatal_project_name_too_long(name, max)
	string_ty	*name;
	int		max;
{
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", name);
	sub_var_set_long(scp, "Number", (long)(name->str_length - max));
	sub_var_optional(scp, "Number");
	fatal_intl(scp, i18n("project $name too long"));
}
