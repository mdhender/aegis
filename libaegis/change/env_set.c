/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate env_sets
 */

#include <ac/stdio.h>

#include <change.h>
#include <change/env_set.h>
#include <env.h>
#include <project.h>


void
change_env_set(cp, with_arch)
	change_ty	*cp;
	int		with_arch;
{
	string_ty	*s;

	/*
	 * set the AEGIS_PROJECT environment cariable
	 */
	env_set("AEGIS_PROJECT", cp->pp->name->str_text);

	/*
	 * set the AEGIS_CHANGE environment cariable
	 */
	if (!cp->bogus)
	{
		char		buffer[20];

		sprintf(buffer, "%ld", magic_zero_decode(cp->number));
		env_set("AEGIS_CHANGE", buffer);
	}
	else
		env_unset("AEGIS_CHANGE");

	/*
	 * set the AEGIS_ARCH environment variable
	 */
	s = change_architecture_name(cp, with_arch);
	if (s)
		env_set("AEGIS_ARCH", s->str_text);
	else
		env_unset("AEGIS_ARCH");
}
