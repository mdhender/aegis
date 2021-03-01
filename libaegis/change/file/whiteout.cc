//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <common/gmatch.h>
#include <common/r250.h>
#include <libaegis/sub.h>
#include <common/trace.h>


static string_ty *
ugly(void)
{
    static char junk[] = "!#$%&()*+,-./:;<=>?@[]^_`{|}~";
    size_t jlen, j;
    char buffer[1024];

    jlen = strlen(junk);
    for (j = 0; j < sizeof(buffer); ++j)
    {
	if ((j % 64) == 63)
	    buffer[j] = '\n';
	else
	    buffer[j] = junk[r250() % jlen];
    }
    buffer[sizeof(buffer) - 1] = '\n';
    return str_n_from_c(buffer, sizeof(buffer));
}


string_ty *
change_file_whiteout(change::pointer cp, string_ty *name)
{
    string_ty	    *result;
    size_t	    j, k;
    pconf_ty        *pconf_data;

    trace(("change_file_template(name = \"%s\")\n{\n", name->str_text));
    assert(cp->reference_count >= 1);
    result = 0;
    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->whiteout_template)
	goto fell_off_end;
    for (j = 0; j < pconf_data->whiteout_template->length; ++j)
    {
	pconf_whiteout_template_ty *ftp;

	ftp = pconf_data->whiteout_template->list[j];
	if (!ftp->pattern)
		continue;
	for (k = 0; k < ftp->pattern->length; ++k)
	{
	    int	            m;
	    string_ty       *s;

	    s = ftp->pattern->list[k];
	    m = gmatch(s->str_text, name->str_text);
	    if (m < 0)
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", name);
		change_fatal(cp, scp, i18n("bad pattern $filename"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    if (m)
	    {
		sub_context_ty	*scp;

		if (!ftp->body)
		{
		    trace(("}\n"));
		    return 0;
		}

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", name);
		sub_var_optional(scp, "File_Name");
		result = substitute(scp, cp, ftp->body);
		sub_context_delete(scp);
		trace(("}\n"));
		return result;
	    }
	}
    }

    //
    // If there are no matching patterns,
    // use a 1KB ugly string.
    //
    fell_off_end:
    result = ugly();
    trace(("}\n"));
    return result;
}
