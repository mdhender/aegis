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
 * MANIFEST: functions to manipulate checks
 */

#include <change.h>
#include <cstate.h>
#include <error.h>/* for assert */
#include <pconf.h>
#include <sub.h>


void
change_check_architectures(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	pconf		pconf_data;
	size_t		j, k;
	int		error_count;

	assert(cp->reference_count >= 1);
	cstate_data = change_cstate_get(cp);
	if (!cstate_data->architecture)
		return; /* should not happen */
	pconf_data = change_pconf_get(cp, 1);
	assert(pconf_data->architecture);
	if (!pconf_data->architecture)
		return; /* should not happen */
	error_count = 0;
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		string_ty	*variant;

		variant = cstate_data->architecture->list[j];
		for (k = 0; k < pconf_data->architecture->length; ++k)
		{
			if
			(
				str_equal
				(
					variant,
					pconf_data->architecture->list[k]->name
				)
			)
				break;
		}
		if (k >= pconf_data->architecture->length)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set(scp, "Name", "%S", variant);
			change_error(cp, scp, i18n("architecture \"$name\" unlisted"));
			sub_context_delete(scp);
			++error_count;
		}
	}
	if (error_count)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set(scp, "Number", "%d", error_count);
		sub_var_optional(scp, "Number");
		change_fatal(cp, scp, i18n("found unlisted architectures"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
}
