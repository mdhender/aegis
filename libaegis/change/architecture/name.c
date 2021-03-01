/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate names
 */

#include <change.h>
#include <error.h> /* for assert */
#include <gmatch.h>
#include <sub.h>
#include <trace.h>
#include <uname.h>


string_ty *
change_architecture_name(change_ty *cp, int with_arch)
{
    trace(("change_architecture_name(cp = %8.8lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    if (!cp->architecture_name)
    {
	pconf		pconf_data;
	long		j;
	string_ty	*result;
	string_ty	*un;

	pconf_data = change_pconf_get(cp, 0);
	assert(pconf_data->architecture);
	un = uname_variant_get();
	result = 0;
	for (j = 0; j < pconf_data->architecture->length; ++j)
	{
	    pconf_architecture ap;

	    ap = pconf_data->architecture->list[j];
	    assert(ap->name);
	    assert(ap->pattern);
	    if (gmatch(ap->pattern->str_text, un->str_text))
	    {
		if (ap->mode == pconf_architecture_mode_forbidden)
		{
		    sub_context_ty	*scp;

		    scp = sub_context_new();
		    sub_var_set_string(scp, "Name", un);
		    change_fatal
		    (
			cp,
			scp,
			i18n("architecture \"$name\" forbidden")
		    );
		    /* NOTREACHED */
		    sub_context_delete(scp);
		}
		result = ap->name;
		break;
	    }
	}
	if (!result && with_arch)
	{
	    sub_context_ty	*scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", un);
	    change_fatal(cp, scp, i18n("architecture \"$name\" unknown"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	cp->architecture_name = result;
    }
    trace_string(cp->architecture_name ? cp->architecture_name->str_text : "");
    trace(("}\n"));
    return cp->architecture_name;
}
