/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate defaults
 */

#include <cattr.h>
#include <change/attributes.h>
#include <error.h>
#include <pconf.h>
#include <project.h>
#include <project/history.h>
#include <trace.h>


void
change_attributes_default(cattr a, project_ty *pp, pconf pc)
{
    trace(("change_attributes_defaults(a = %08lX, pp = %08lX, pc = %08lX)\n{\n",
	(long)a, (long)pp, (long)pc));
    if
    (
	a->cause == change_cause_internal_improvement
    ||
	a->cause == change_cause_external_improvement
    )
    {
	if (!(a->mask & cattr_test_exempt_mask))
	{
	    a->test_exempt = 1;
	    a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
	    a->test_baseline_exempt = 1;
	    a->mask |= cattr_test_baseline_exempt_mask;
	}
	if (!(a->mask & cattr_regression_test_exempt_mask))
	{
	    a->regression_test_exempt = 0;
	    a->mask |= cattr_regression_test_exempt_mask;
	}
    }
    else
    {
	if (!(a->mask & cattr_regression_test_exempt_mask))
	{
	    a->regression_test_exempt = 1;
	    a->mask |= cattr_regression_test_exempt_mask;
	}
    }
    if (!(a->mask & cattr_test_exempt_mask))
    {
	a->test_exempt = project_default_test_exemption_get(pp);
	a->mask |= cattr_test_exempt_mask;
    }
    if (!(a->mask & cattr_test_baseline_exempt_mask))
    {
	a->test_baseline_exempt = project_default_test_exemption_get(pp);
	a->mask |= cattr_test_baseline_exempt_mask;
    }

    if (!a->architecture)
	a->architecture =
	    (cattr_architecture_list)cattr_architecture_list_type.alloc();
    assert(pc->architecture);
    assert(pc->architecture->length);
    if (!a->architecture->length)
    {
	long		j;

	for (j = 0; j < pc->architecture->length; ++j)
	{
	    type_ty	    *type_p;
	    string_ty	    **str_p;
	    pconf_architecture pca;

	    pca = pc->architecture->list[j];
	    if (pca->mode != pconf_architecture_mode_required)
		continue;
	    str_p =
		cattr_architecture_list_type.list_parse
		(
		    a->architecture,
		    &type_p
		);
	    assert(type_p==&string_type);
	    *str_p = str_copy(pca->name);
	}
    }
    trace(("}\n"));
}
