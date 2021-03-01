//
//	aegis - project change supervisor
//	Copyright (C) 2000, 2002-2008 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/cattr.h>
#include <libaegis/change/attributes.h>
#include <libaegis/pconf.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>


void
change_attributes_default(cattr_ty *a, project_ty *pp, pconf_ty *pc)
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
	    a->test_exempt = true;
	    a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
	    a->test_baseline_exempt = true;
	    a->mask |= cattr_test_baseline_exempt_mask;
	}
    }
    if (!(a->mask & cattr_test_exempt_mask))
    {
	a->test_exempt = project_default_test_exemption_get(pp);
	a->mask |= cattr_test_exempt_mask;
    }
    if (!(a->mask & cattr_test_baseline_exempt_mask))
    {
	a->test_baseline_exempt =
	    project_default_test_exemption_get(pp);
	a->mask |= cattr_test_baseline_exempt_mask;
    }
    if (!(a->mask & cattr_regression_test_exempt_mask))
    {
	a->regression_test_exempt =
	    project_default_test_regression_exemption_get(pp);
	a->mask |= cattr_regression_test_exempt_mask;
    }

    if (!a->architecture)
    {
	a->architecture =
	    (cattr_architecture_list_ty *)cattr_architecture_list_type.alloc();
    }
    assert(pc->architecture);
    assert(pc->architecture->length);
    if (!a->architecture->length)
    {
	for (size_t j = 0; j < pc->architecture->length; ++j)
	{
	    pconf_architecture_ty *pca = pc->architecture->list[j];
	    if (pca->mode != pconf_architecture_mode_required)
		continue;
	    meta_type *type_p = 0;
	    string_ty **str_p =
		(string_ty **)
		cattr_architecture_list_type.list_parse
		(
		    a->architecture,
		    &type_p
		);
	    assert(type_p == &string_type);
	    *str_p = str_copy(pca->name);
	}
    }
    trace(("}\n"));
}
