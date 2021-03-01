//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2003-2008 Peter Miller
//	Copyright (C) 2007, 2008 Walter Franzini
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

#include <common/ac/stdio.h>
#include <common/ac/string.h>

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <common/env.h>
#include <common/nstring.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change_env_set(change::pointer cp, int with_arch)
{
    //
    // Set environment variables based on project_specific attributes,
    // if specified.
    //
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if
    (
        pconf_data->project_specific
    &&
        !cp->project_specific_setenv_performed_get()
    )
    {
	for (size_t j = 0; j < pconf_data->project_specific->length; ++j)
	{
	    attributes_ty *ap =
		pconf_data->project_specific->list[j];
	    if
	    (
		ap->name
	    &&
		ap->name->str_length > 7
	    &&
		0 == strncasecmp(ap->name->str_text, "setenv:", 7)
	    )
	    {
		const char *name = ap->name->str_text + 7;
		if (ap->value && ap->value->str_length)
                {
                    sub_context_ty *scp = sub_context_new();
                    nstring env_value(substitute(scp, cp, ap->value));
                    env_set(name, env_value.c_str());
                    sub_context_delete(scp);
                }
		else
		    env_unset(name);
	    }
	}
        cp->project_specific_setenv_performed_set();
    }

    //
    // Set the LINES and COLS environment variables.
    //
    env_set_page();

    //
    // set the AEGIS_PROJECT environment cariable
    //
    env_set("AEGIS_PROJECT", cp->pp->name_get()->str_text);

    //
    // set the AEGIS_CHANGE environment cariable
    //
    if (!cp->bogus)
	env_setf("AEGIS_CHANGE", "%ld", magic_zero_decode(cp->number));
    else
	env_unset("AEGIS_CHANGE");

    //
    // set the AEGIS_ARCH environment variable
    //
    string_ty *s = change_architecture_name(cp, with_arch);
    if (s)
	env_set("AEGIS_ARCH", s->str_text);
    else
	env_unset("AEGIS_ARCH");
}
