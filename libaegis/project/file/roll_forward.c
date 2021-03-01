/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate roll_forwards
 */

#include <change.h>
#include <change/file.h>
#include <change/branch.h>
#include <cstate.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <project/file/roll_forward.h>
#include <symtab.h>
#include <trace.h>


static file_event_list_ty *file_event_list_new _((void));

static file_event_list_ty *
file_event_list_new()
{
	file_event_list_ty *felp;
	felp = mem_alloc(sizeof(file_event_list_ty));
	felp->length = 0;
	felp->maximum = 0;
	felp->item = 0;
	return felp;
}


#if 0

static void file_event_list_delete _((file_event_list_ty *felp));

static void
file_event_list_delete(felp)
	file_event_list_ty *felp;
{
	if (felp->item)
		mem_free(felp->item);
	felp->length = 0;
	felp->maximum = 0;
	felp->item = 0;
}

#endif


static void file_event_list_append _((file_event_list_ty *felp, time_t when,
	change_ty *cp));

static void
file_event_list_append(felp, when, cp)
	file_event_list_ty *felp;
	time_t		when;
	change_ty	*cp;
{
	file_event_ty	*fep;

	/*
	 * The event should be the last on the list.  If it isn't,
	 * then a child branch supercedes the sequence.
	 */
	while
	(
		felp->length > 0
	&&
		felp->item[felp->length - 1].when >= when
	)
		felp->length--;

	/*
	 * Drop the event onto the end of the list.
	 */
	if (felp->length >= felp->maximum)
	{
		size_t		nbytes;

		felp->maximum = felp->maximum * 2 + 4;
		nbytes = felp->maximum * sizeof(felp->item[0]);
		felp->item = mem_change_size(felp->item, nbytes);
	}
	fep = felp->item + felp->length++;
	fep->when = when;
	fep->cp = cp;
}


static symtab_ty *stp;


static void recapitulate2 _((project_ty *pp, time_t limit, int detailed));

static void
recapitulate2(pp, limit, detailed)
	project_ty	*pp;
	time_t		limit;
	int		detailed;
{
	change_ty	*pcp;
	cstate		cstate_data;
	cstate_branch	bp;
	size_t		j, k;

	trace(("recapitulate2(pp = %08lX, limit = %ld, detailed = %d)\n{\n",
		(long)pp, (long)limit, detailed));
	pcp = project_change_get(pp);
	cstate_data = change_cstate_get(pcp);
	bp = cstate_data->branch;
	assert(bp);
	if (!bp)
	{
		trace(("}\n"));
		return;
	}
	if (!bp->history)
	{
		trace(("}\n"));
		return;
	}

	/*
	 * Work our way down the branch, squirreling away each child
	 * change as we see it.
	 */
	for (j = 0; j < bp->history->length; ++j)
	{
		long		change_number;
		change_ty	*cp;
		cstate_branch_history hp;
		cstate_history	chp;

		hp = bp->history->list[j];
		assert(hp);
		change_number = hp->change_number;
		cp = change_alloc(pp, change_number);
		change_bind_existing(cp);
		cstate_data = change_cstate_get(cp);

		/*
		 * If the change hasn't been completed yet, don't put
		 * it into the history.  It isn't relevant yet.
		 */
		if (cstate_data->state != cstate_state_completed)
		{
			change_free(cp);
			continue;
		}

		/*
		 * If the change was completed after the time limit,
		 * ignore the change.  Also, any later changes are
		 * irrelevant, too.
		 */
		assert(cstate_data->history);
		assert(cstate_data->history->length > 1);
		if (!cstate_data->history || cstate_data->history->length < 1)
		{
			change_free(cp);
			continue;
		}
		chp =
			cstate_data->history->list
			[
				cstate_data->history->length - 1
			];
		assert(chp);
		if (chp->when > limit)
		{
			change_free(cp);
			break;
		}

		/*
		 * The detailed variant descends into each of the
		 * completed branches.
		 */
		if (detailed && change_was_a_branch(cp))
		{
			project_ty	*pp2;

			pp2 = project_bind_branch(pp, cp);
			recapitulate2(pp2, limit, detailed);

			/*
			 * Don't add this branch to the history.  In the
			 * detailed version we aren't interested in branch
			 * integrations - they don't change anything.
			 */
			continue;

			/*
			 * I suspect it is a memory leak that we don't
			 * project_free(pp2) but there are so many other
			 * leaks, I'm not going to worry.
			 */
		}

		/*
		 * For each of the files, add the change to the end of
		 * the sequence.
		 */
		for (k = 0; ; ++k)
		{
			fstate_src	src;
			file_event_list_ty *felp;

	    		src = change_file_nth(cp, k);
	    		if (!src)
	    			break;
			felp = symtab_query(stp, src->file_name);
			if (!felp)
			{
				felp = file_event_list_new();
				symtab_assign(stp, src->file_name, felp);
			}
			file_event_list_append(felp, chp->when, cp);
		}
	}
	trace(("}\n"));
}


static void recapitulate _((project_ty *pp, time_t limit, int detailed));

static void
recapitulate(pp, limit, detailed)
	project_ty	*pp;
	time_t		limit;
	int		detailed;
{
	trace(("recapitulate(pp = %08lX, limit = %ld, detailed = %d)\n{\n",
		(long)pp, (long)limit, detailed));
	if (pp->parent)
		recapitulate(pp->parent, limit, detailed);
	recapitulate2(pp, limit, detailed);
	trace(("}\n"));
}


void
project_file_roll_forward(pp, limit, detailed)
	project_ty	*pp;
	time_t		limit;
	int		detailed;
{
	trace(("project_file_roll_forward(pp = %08lX, limit = %ld, "
		"detailed = %d)\n{\n", (long)pp, (long)limit, detailed));
	assert(!stp);
	stp = symtab_alloc(1000);
	recapitulate(pp, limit, !!detailed);
	trace(("}\n"));
}


file_event_list_ty *
project_file_roll_forward_get(filename)
	string_ty	*filename;
{
	file_event_list_ty *result;

	trace(("project_file_roll_forward_get(%s)\n{\n", filename->str_text));
	result = symtab_query(stp, filename);
	trace(("return %08lX\n", (long)result));
	trace(("}\n"));
	return result;
}
