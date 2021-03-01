/*
 *	aegis - project change supervisor
 *	Copyright (C) 2000 Peter Miller;
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
 * MANIFEST: functions to manipulate copys
 */

#include <cattr.h>
#include <change/attributes.h>
#include <cstate.h>
#include <error.h>
#include <trace.h>


void
change_attributes_copy(a, s)
	cattr	a;
	cstate	s;
{
	trace(("cattr_copy()\n{\n"));
	if (!a->description && s->description)
	{
		a->description = str_copy(s->description);
		a->mask |= cattr_description_mask;
	}
	if (!a->brief_description && s->brief_description)
	{
		a->brief_description = str_copy(s->brief_description);
		a->mask |= cattr_brief_description_mask;
	}
	if (!(a->mask & cattr_cause_mask))
	{
		a->cause = s->cause;
		a->mask |= cattr_cause_mask;
	}
	if (!(a->mask & cattr_regression_test_exempt_mask))
	{
		a->regression_test_exempt = s->regression_test_exempt;
		a->mask |= cattr_regression_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_exempt_mask))
	{
		a->test_exempt = s->test_exempt;
		a->mask |= cattr_test_exempt_mask;
	}
	if (!(a->mask & cattr_test_baseline_exempt_mask))
	{
		a->test_baseline_exempt = s->test_baseline_exempt;
		a->mask |= cattr_test_baseline_exempt_mask;
	}

	if (!a->architecture)
		a->architecture =
			(cattr_architecture_list)
			cattr_architecture_list_type.alloc();
	if (!a->architecture->length)
	{
		long		j;

		for (j = 0; j < s->architecture->length; ++j)
		{
			type_ty		*type_p;
			string_ty	**str_p;

			str_p =
				cattr_architecture_list_type.list_parse
				(
					a->architecture,
					&type_p
				);
			assert(type_p == &string_type);
			*str_p = str_copy(s->architecture->list[j]);
		}
	}

	if (s->copyright_years)
	{
		if (!a->copyright_years)
			a->copyright_years =
				(cattr_copyright_years_list)
				cattr_copyright_years_list_type.alloc();
		if (!a->copyright_years->length)
		{
			long		j;

			for (j = 0; j < s->copyright_years->length; ++j)
			{
				type_ty		*type_p;
				long		*int_p;

				int_p =
					cattr_copyright_years_list_type.list_parse
					(
						a->copyright_years,
						&type_p
					);
				assert(type_p == &integer_type);
				*int_p = s->copyright_years->list[j];
			}
		}
	}
	if (!a->version_previous && s->version_previous)
	{
		a->version_previous = str_copy(s->version_previous);
		a->mask |= cattr_version_previous_mask;
	}
	trace(("}\n"));
}