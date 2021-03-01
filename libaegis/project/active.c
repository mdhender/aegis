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
 * MANIFEST: functions to manipulate actives
 */

#include <change.h>
#include <change/branch.h>
#include <project.h>
#include <project/active.h>
#include <project/history.h>
#include <sub.h>


int
project_active(pp, active_branch_ok)
	project_ty	*pp;
	int		active_branch_ok;
{
	long		j;
	long		change_number;
	change_ty	*cp;
	int		active;
	project_ty	*p2;
	cstate		cstate_data;
	int		n;

	active = 0;
	for (j = 0; ; ++j)
	{
		if (!project_change_nth(pp, j, &change_number))
			break;
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		if (change_is_a_branch(cp))
		{
			p2 = project_bind_branch(pp, change_copy(cp));
			n = project_active(p2, active_branch_ok);
			active += n;
			project_free(p2);

			if (!active_branch_ok && !n)
			{
				/*
				 * active_branch_ok is used when deleting
				 * whole projects.
			         *
				 * Logically, we should say this all
				 * the time, however only saying it if
				 * there are no active changes seems
				 * to meet user's expectations better.
				 */
				change_error(cp, 0, i18n("outstanding change"));
				++active;
			}
		}
		else
		{
			cstate_data = change_cstate_get(cp);
			switch (cstate_data->state)
			{
			case cstate_state_awaiting_development:
				/*
				 * active_branch_ok is used when deleting
				 * whole projects.
				 */
				if (active_branch_ok)
					break;
				/* fall through... */

			case cstate_state_being_developed:
			case cstate_state_awaiting_review:
			case cstate_state_being_reviewed:
			case cstate_state_awaiting_integration:
			case cstate_state_being_integrated:
				change_error(cp, 0, i18n("outstanding change"));
				++active;
				break;

			case cstate_state_completed:
				break;
			}
		}
		change_free(cp);
	}
	return active;
}


void
project_active_check(pp, brok)
	project_ty	*pp;
	int		brok;
{
	int		num_err;
	sub_context_ty	*scp;

	num_err = project_active(pp, brok);
	if (num_err)
	{
		scp = sub_context_new();
		sub_var_set_long(scp, "Number", num_err);
		sub_var_optional(scp, "Number");
		project_fatal(pp, scp, i18n("outstanding changes"));
		/* NOTREACHED */
	}
}


void
project_active_check_branch(cp, brok)
	change_ty	*cp;
	int		brok;
{
	project_ty	*pp;

	if (!change_is_a_branch(cp))
		return;
	pp = project_bind_branch(cp->pp, change_copy(cp));
	project_active_check(pp, brok);
	project_free(pp);
}
