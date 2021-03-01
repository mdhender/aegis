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
 * MANIFEST: functions to manipulate templates
 */

#include <change.h>
#include <error.h> /* for assert */
#include <gmatch.h>
#include <sub.h>
#include <trace.h>


string_ty *
change_file_template(cp, name)
	change_ty	*cp;
	string_ty	*name;
{
	string_ty	*result;
	size_t		j, k;
	pconf		pconf_data;

	trace(("change_file_template(name = \"%s\")\n{\n"/*}*/,
		name->str_text));
	assert(cp->reference_count >= 1);
	result = 0;
	pconf_data = change_pconf_get(cp, 0);
	if (!pconf_data->file_template)
		goto done;
	for (j = 0; j < pconf_data->file_template->length; ++j)
	{
		pconf_file_template ftp;

		ftp = pconf_data->file_template->list[j];
		if (!ftp->pattern)
			continue;
		for (k = 0; k < ftp->pattern->length; ++k)
		{
			int		m;
			string_ty	*s;

			s = ftp->pattern->list[k];
			m = gmatch(s->str_text, name->str_text);
			if (m < 0)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp,"File_Name", "%S", s);
				change_fatal(cp, scp, i18n("bad pattern $filename"));
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			if (m)
				break;
		}
		if (k < ftp->pattern->length)
		{
			result = str_copy(ftp->body);
			break;
		}
	}

	/*
	 * here for all exits
	 */
	done:
	if (result)
	{
		sub_context_ty	*scp;
		string_ty	*s;

		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", name);
		sub_var_optional(scp, "File_Name");
		s = substitute(scp, cp, result);
		sub_context_delete(scp);
		str_free(result);
		result = s;
	}
	trace((/*{*/"}\n"));
	return result;
}
