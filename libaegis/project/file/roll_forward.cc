//
//	aegis - project change supervisor
//	Copyright (C) 2001-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate roll_forwards
//

#pragma implementation "project_file_roll_forward"

#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/list.h>
#include <cstate.h>
#include <error.h> // for assert
#include <mem.h>
#include <now.h>
#include <project/file/roll_forward.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <zero.h>


project_file_roll_forward::~project_file_roll_forward()
{
}


static file_event_list_ty *
file_event_list_new(void)
{
    file_event_list_ty *felp;

    trace(("file_event_list_new()\n{\n"));
    felp = (file_event_list_ty *)mem_alloc(sizeof(file_event_list_ty));
    felp->length = 0;
    felp->maximum = 0;
    felp->item = 0;
    trace(("return %08lX;\n", (long)felp));
    trace(("}\n"));
    return felp;
}


static void
file_event_list_append(file_event_list_ty *felp, time_t when, change_ty	*cp,
    fstate_src_ty *src)
{
    file_event_ty	*fep;

    //
    // The event should be the last on the list.  If it isn't,
    // then a child branch supercedes the sequence.
    //
    trace(("file_event_list_append(felp = %08lX, when = %ld \"%.24s\", "
	"cp = %08lX)\n{\n", (long)felp, (long)when, ctime(&when), (long)cp));
    while (felp->length > 0 && felp->item[felp->length - 1].when >= when)
	felp->length--;

    //
    // Drop the event onto the end of the list.
    //
    if (felp->length >= felp->maximum)
    {
	size_t		nbytes;

	felp->maximum = felp->maximum * 2 + 4;
	nbytes = felp->maximum * sizeof(felp->item[0]);
	felp->item = (file_event_ty *)mem_change_size(felp->item, nbytes);
    }
    fep = felp->item + felp->length++;
    fep->when = when;
    fep->cp = cp;
    fep->src = src;
    trace(("}\n"));
}


static int
possibly_broken_by_aeimport(change_ty *cp)
{
    fstate_src_ty   *src;
    cstate_ty       *cstate_data;
    cstate_history_ty *chp;

    //
    // Older versions of aeimport had a bug, and the very
    // first change on the imported branch had an honest time
    // stamp, the date of the aeimport run, but it confuses
    // this function mightily.  (It no longer does this, it
    // makes up a timestamp earlier than the first imported
    // change set.)  As a work around, ignore the limit for
    // the first change on such a branch.
    //
    if (change_file_nth(cp, 1, view_path_first))
	return 0;
    src = change_file_nth(cp, 0, view_path_first);
    if (!src)
	return 0;
    switch (src->action)
    {
    case file_action_create:
	if (src->usage != file_usage_config)
	    return 0;
	break;

    case file_action_modify:
    case file_action_remove:
    case file_action_insulate:
    case file_action_transparent:
#ifndef DEBUG
    default:
#endif
	return 0;
    }

    //
    // Check that the history is the shape we expect.
    //
    cstate_data = change_cstate_get(cp);
    if(!cstate_data->history)
	return 0;
    if (cstate_data->history->length < 4)
	return 0;
    chp = cstate_data->history->list[cstate_data->history->length - 1];
    if (chp->what != cstate_history_what_integrate_pass)
	return 0;
    if (chp->when - cstate_data->history->list[0]->when >= 10)
	return 0;

    //
    // Tickle the time, so that it is marginally valid.
    //
    // This won't corrupt anything, because this change will not be
    // written out.
    //
    chp->when = 24*60*60;
    return 1;
}


static change_list_ty walk_these_branches;


static change_list_ty *
change_list_get(project_ty *pp, time_t limit)
{
    change_ty       *pcp;
    cstate_ty       *cstate_data;
    cstate_branch_ty *bp;
    size_t	    j;
    change_list_ty  *clp;

    trace(("change_list_get(pp = %08lX, limit = %ld \"%.24s\")\n{\n",
	(long)pp, (long)limit, ctime(&limit)));
    trace(("project \"%s\"\n", project_name_get(pp)->str_text));
    clp = new change_list_ty();

    pcp = project_change_get(pp);
    cstate_data = change_cstate_get(pcp);
    bp = cstate_data->branch;
    assert(bp);
    if (!bp)
    {
	trace(("}\n"));
	return clp;
    }

    //
    // Work our way down the branch, squirreling away each child
    // change as we see it.
    //
    if (bp->history)
    {
	for (j = 0; j < bp->history->length; ++j)
	{
	    long            change_number;
	    change_ty       *cp;
	    cstate_branch_history_ty *hp;
	    cstate_history_ty *chp;

	    hp = bp->history->list[j];
	    assert(hp);
	    change_number = hp->change_number;
	    cp = change_alloc(pp, change_number);
	    trace(("cp = %08lX;\n", (long)cp));
	    change_bind_existing(cp);
	    cstate_data = change_cstate_get(cp);

	    //
	    // If the change hasn't been completed yet, don't put
	    // it into the history.  It isn't relevant yet.
	    //
	    // We are walking the *history* list.
	    // All these changes are supposed to be completed.
	    //
	    assert(cstate_data->state == cstate_state_completed);
	    if (cstate_data->state != cstate_state_completed)
	    {
		change_free(cp);
		continue;
	    }

	    //
	    // If the change was completed after the time limit,
	    // ignore the change.  Also, any later changes are
	    // irrelevant, too.
	    //
	    assert(cstate_data->history);
	    assert(cstate_data->history->length > 1);
	    if (!cstate_data->history || cstate_data->history->length < 1)
	    {
		change_free(cp);
		continue;
	    }
	    chp = cstate_data->history->list[cstate_data->history->length - 1];
	    trace(("when = %ld \"%.24s\"\n", (long)chp->when,
		ctime(&chp->when)));
	    assert(chp);

	    //
	    // Older versions of aeimport had a bug, and the very
	    // first change on the imported branch had an honest time
	    // stamp, the date of the aeimport run, but it confuses
	    // this function mightily.  (It no longer does this, it
	    // makes up a timestamp earlier than the first imported
	    // change set.)  As a work around, ignore the limit for
	    // the first change on such a branch.
	    //
	    if
	    (
		j != 0
	    ||
		change_number != 10
	    ||
		!possibly_broken_by_aeimport(cp)
	    )
	    {
		if (chp->when > limit)
		{
		    trace(("toss this one, and all the rest\n"));
		    change_free(cp);
		    break;
		}
	    }

	    trace(("keep this one\n"));
	    clp->append(cp);
	}
    }

    //
    // Also chase down the exception branch list because there may be
    // some active branches that should be walked, and they will *not*
    // be on the history list.
    //
    // They go on the end of the list, because they aren't completed yet.
    // When they are eventually completed, they will be at this position
    // in the list.
    //
    for (j = 0; j < walk_these_branches.length; ++j)
    {
	change_ty       *cp;

	cp = walk_these_branches.item[j];
	trace(("pp = %08lX\n", (long)pp));
	trace(("cp->pp = %08lX\n", (long)cp->pp));
	trace(("same = %d\n", str_equal(project_name_get(cp->pp),
	    project_name_get(pp))));
	if (str_equal(project_name_get(cp->pp), project_name_get(pp)))
	    clp->append(cp);
    }

    trace(("return %08lX;\n", (long)clp));
    trace(("}\n"));
    return clp;
}


struct playback_ty
{
    project_ty      *pp;
    change_list_ty  *clp;
    size_t          position;
    size_t          pushed;
    symtab_ty       *files;
};


static void
playback_constructor(playback_ty *pbp, time_t limit, project_ty *pp)
{
    trace(("playback_constructor(pbp = %08lX, limit = %ld \"%.24s\", "
	"pp = %08lX)\n{\n", (long)pbp, (long)limit, ctime(&limit), (long)pp));
    pbp->pp = pp;
    pbp->clp = change_list_get(pp, limit);
    pbp->position = 0;
    pbp->pushed = 0;
    pbp->files = new symtab_ty(5);
    trace(("}\n"));
}


static void
playback_destructor(playback_ty *pbp)
{
    trace(("playback_destructor(pbp = %08lX)\n{\n", (long)pbp));
    if (pbp->clp)
	delete pbp->clp;
    if (pbp->files)
	delete pbp->files;
    pbp->pp = 0;
    pbp->clp = 0;
    pbp->position = 0;
    pbp->pushed = 0;
    pbp->files = 0;
    trace(("}\n"));
}


static time_t
playback_when(playback_ty *pbp)
{
    if (!pbp || !pbp->clp)
	return time_max();
    if (pbp->position >= pbp->clp->length)
    {
	change_ty       *cp;

	cp = project_change_get(pbp->pp);
	return change_completion_timestamp(cp);
    }
    return change_completion_timestamp(pbp->clp->item[pbp->position]);
}


struct playback_list_ty
{
    size_t          length;
    size_t          maximum;
    playback_ty     *item;
};


static void
playback_list_constructor(playback_list_ty *pblp)
{
    trace(("playback_list_constructor(pblp = %08lX)\n{\n", (long)pblp));
    pblp->length = 0;
    pblp->maximum = 0;
    pblp->item = 0;
    trace(("}\n"));
}


static void
playback_list_destructor(playback_list_ty *pblp)
{
    trace(("playback_list_destructor(pblp = %08lX)\n{\n", (long)pblp));
    if (pblp->item)
    {
	size_t          j;

	for (j = 0; j < pblp->length; ++j)
	    playback_destructor(pblp->item + j);
	mem_free(pblp->item);
    }
    pblp->length = 0;
    pblp->maximum = 0;
    pblp->item = 0;
    trace(("}\n"));
}


static void
playback_list_push(playback_list_ty *pblp, time_t limit, project_ty *pp)
{
    playback_ty     *pbp;

    trace(("playback_list_push(pblp = %08lX, limit = %ld \"%.24s\", "
	"pp = %08lX)\n{\n", (long)pblp, (long)limit, ctime(&limit), (long)pp));
    trace(("project \"%s\"\n", project_name_get(pp)->str_text));
    if (pblp->length >= pblp->maximum)
    {
	size_t          nbytes;

	pblp->maximum = pblp->maximum * 2 + 8;
	nbytes = pblp->maximum * sizeof(pblp->item[0]);
	pblp->item = (playback_ty *)mem_change_size(pblp->item, nbytes);
    }
    pbp = pblp->item + pblp->length++;
    trace(("pbp = %08lX\n", (long)pbp));
    playback_constructor(pbp, limit, pp);
    trace(("}\n"));
}


static void
playback_list_pop(playback_list_ty *pblp)
{
    trace(("playback_list_pop(pblp = %08lX)\n{\n", (long)pblp));
    assert(pblp->length > 0);
    if (pblp->length > 0)
    {
	playback_ty     *pbp;

	pblp->length--;
	pbp = pblp->item + pblp->length;
	playback_destructor(pbp);
    }
    trace(("}\n"));
}


static void
playback_list_recinit(playback_list_ty *pblp, time_t limit, project_ty *pp)
{
    trace(("playback_list_recinit(pblp = %08lX, limit = %ld \"%.24s\", "
	"pp = %08lX)\n{\n", (long)pblp, (long)limit, ctime(&limit), (long)pp));
    while (pp->parent)
    {
	change_ty       *cp;

	//
        // Completed changes will always be placed on the list by
        // change_list_get().
	//
        // Active branches need to be explicitly added, which is what
        // the walk_these_branches list is for.
	//
	// Completed ancestor branches, however, will have a completion
	// time stamp after the time limit, and so they, too, need to
	// be added to this list.
	//
	cp = project_change_get(pp);
	walk_these_branches.append(cp);
	pp = pp->parent;
    }
    playback_list_push(pblp, limit, pp);
    trace(("}\n"));
}


#if 0

static void
playback_trace_real(const char *file, int line, playback_ty *pbp)
{
    change_ty       *cp;
    time_t          when;

    trace_where(file, line);
    trace_printf("playback %08lX = { ", (long)pbp);
    if (pbp->position >= pbp->clp->length)
    {
	cp = project_change_get(pbp->pp);
	trace_printf("project \"%s\" END\n",
	    project_name_get(pbp->pp)->str_text);
    }
    else
    {
	cp = pbp->clp->item[pbp->position];
	trace_printf("project \"%s\", change %ld, delta %ld",
	    project_name_get(cp->pp)->str_text,
	    magic_zero_decode(cp->number), change_delta_number_get(cp));
    }
    when = change_completion_timestamp(cp);
    trace_printf(", time %ld = %.24s}\n", (long)when, ctime(&when));
}

#define playback_trace(x) playback_trace_real(__FILE__, __LINE__, (x))

#else

#define playback_trace(x)

#endif


static playback_ty *
playback_list_next(playback_list_ty *pblp, size_t *index_p)
{
    playback_ty     *pbp;
    size_t          idx;
    size_t          idx2;
    time_t          earliest_time;

    //
    // Find *any* item on the playback list.
    //
    trace(("playback_list_next(pblp = %08lX)\n{\n", (long)pblp));
    for (idx = 0; ; ++idx)
    {
	if (idx >= pblp->length)
	{
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    return 0;
	}
	pbp = pblp->item + idx;
	if (pbp->position < pbp->clp->length)
	    break;
    }
    earliest_time = playback_when(pbp);
    playback_trace(pbp);

    //
    // Now that we have found one, see if there is another one which is
    // earlier than first one we found.
    //
    assert(pbp);
    for (idx2 = idx + 1; idx2 < pblp->length; ++idx2)
    {
	playback_ty     *pbp2;
	time_t		second_time;

	pbp2 = pblp->item + idx2;
	if (pbp2->position >= pbp2->clp->length)
	    continue;
	playback_trace(pbp2);
	second_time = playback_when(pbp2);
	if (second_time <= earliest_time)
	{
	    //
	    // That "or equals" is important - we want the one the
	    // closest to the top of the stack.
	    //
	    pbp = pbp2;
	    idx = idx2;
	    earliest_time = second_time;
	}
    }
    *index_p = idx;
    trace(("return %08lX;\n", (long)pbp));
    trace(("}\n"));
    return pbp;
}


static time_t
branch_start_time(project_ty *pp)
{
    change_ty       *cp;
    cstate_ty       *cstate_data;
    cstate_history_ty *hp;

    cp = project_change_get(pp);
    cstate_data = change_cstate_get(cp);
    if (!cstate_data)
	return 1;
    if (!cstate_data->history)
	return 1;
    if (!cstate_data->history->length)
	return 1;
    hp = cstate_data->history->list[0];
    if (!hp)
	return 1;
    return hp->when;
}


static int
branch_is_completed(project_ty *pp)
{
    return change_is_completed(project_change_get(pp));
}


static time_t
branch_finish_time(project_ty *pp)
{
    change_ty       *cp;
    cstate_ty       *cstate_data;
    cstate_history_ty *hp;

    cp = project_change_get(pp);
    cstate_data = change_cstate_get(cp);
    if (!cstate_data)
    {
	barf:
	return time_max();
    }
    if (!cstate_data->history)
	goto barf;
    if (!cstate_data->history->length)
	goto barf;
    hp = cstate_data->history->list[cstate_data->history->length - 1];
    if (!hp)
	goto barf;
    if (hp->what != cstate_history_what_integrate_pass)
	goto barf;
    return hp->when;
}


static string_ty *
uuid_or_filename(fstate_src_ty *src)
{
    assert(src);
    assert(src->file_name);
    return (src->uuid ? src->uuid : src->file_name);
}


time_t
project_file_roll_forward::recapitulate(project_ty *pp, time_t limit,
    int detailed)
{
    playback_list_ty stack;
    size_t	    j;
    time_t	    result;
    change_list_ty  *clp;
    time_t          other_time;

    trace(("recapitulate(pp = %08lX, limit = %ld, detailed = %d)\n{\n",
	(long)pp, (long)limit, detailed));
    result = 0;
    playback_list_constructor(&stack);
    playback_list_recinit(&stack, limit, pp);

    //
    // Check that the time requested makes sense.
    //
    other_time = branch_start_time(pp);
    if (limit < other_time)
    {
	sub_context_ty *scp;

	trace(("branch start %ld = %.24s\n", (long)other_time,
	    ctime(&other_time)));
	scp = sub_context_new();
	sub_var_set_time(scp, "Number", limit);
	sub_var_set_time(scp, "Number2", other_time);
	sub_var_optional(scp, "Number2");
	project_error(pp, scp, i18n("time $number before branch"));
	sub_context_delete(scp);
    }
    if (branch_is_completed(pp))
    {
	other_time = branch_finish_time(pp);
	if (limit > other_time)
	{
	    sub_context_ty *scp;

	    trace(("branch finish %ld = %.24s\n", (long)other_time,
		ctime(&other_time)));
	    scp = sub_context_new();
	    sub_var_set_time(scp, "Number", limit);
	    sub_var_set_time(scp, "Number2", other_time);
	    sub_var_optional(scp, "Number2");
	    project_error(pp, scp, i18n("time $number after branch"));
	    sub_context_delete(scp);
	}
    }

    //
    // Work our way along all the branches in parallel, squirreling away
    // each relevant change as we see it.
    //
    for (;;)
    {
	playback_ty     *pbp;
	size_t          idx;
	change_ty       *cp;
	time_t          pb_when;

	//
	// If the top of the stack is a branch, and we are a detailed
	// report, we need to push the branch onto the stack.
	//
	if (stack.length)
	{
	    for (;;)
	    {
		pbp = stack.item + stack.length - 1;
		clp = pbp->clp;
		trace(("%ld/%ld/%ld\n", (long)pbp->position, (long)pbp->pushed,
		    (long)clp->length));
		if (pbp->position >= clp->length)
		    break;
		cp = clp->item[pbp->position];
		if (!change_was_a_branch(cp))
		    break;
		if (pbp->position < pbp->pushed)
		    break;
		trace(("project \"%s\", change %ld, delta %ld\n",
		    project_name_get(cp->pp)->str_text,
		    magic_zero_decode(cp->number),
		    change_delta_number_get(cp)));
		if
		(
		    (detailed && change_is_completed(cp))
		||
		    walk_these_branches.member_p(cp)
		)
		{
		    project_ty      *pp2;

		    pp2 = project_bind_branch(cp->pp, cp);
		    playback_list_push(&stack, limit, pp2);
		}

		//
		// Advance past this change, otherwise we get stuck
		// in an infinite loop, because it keeps reappearing
		// on the top of the stack.
		//
		if (detailed || !change_is_completed(cp))
		    pbp->position++;
		else
		    pbp->pushed = pbp->position + 1;
	    }
	}

    	//
	// Look at the stack and see which ancestor has the next event.
	//
	// We are replaying all of the ancestors in parallel, to simulate
	// what really happened.
	//
	pbp = playback_list_next(&stack, &idx);
	trace(("pbp = %08lX;\n", (long)pbp));
	if (!pbp)
	    break;
	playback_trace(pbp);

	pb_when = playback_when(pbp);
	trace(("when = %ld \"%.24s\"\n", (long)pb_when, ctime(&pb_when)));
	if (result < pb_when)
	{
	    result = pb_when;
	    trace(("result = %ld \"%.24s\"\n", (long)result, ctime(&result)));
	}

	//
	// Now that we know what happened next,
	// simulate the change.
	//
	trace(("pbp->clp = %08lX;\n", (long)pbp->clp));
	trace(("pbp->clp->length = %ld;\n", (long)pbp->clp->length));
	trace(("pbp->position = %ld;\n", (long)pbp->position));
	assert(pbp->position < pbp->clp->length);
	cp = pbp->clp->item[pbp->position];
	assert(cp);
	trace(("project \"%s\", change %ld, delta %ld\n",
	    project_name_get(cp->pp)->str_text,
	    magic_zero_decode(cp->number), change_delta_number_get(cp)));
	last_change = cp;

	//
	// For each file in the change, look down the view path and see
	// if the effects of the change are visable.
	//
	trace(("cp = %08lX;\n", (long)cp));
	if
	(
	    //
	    // Changes in walk_these_branches may be outside the
	    // time limit.  Branches we are walking because they are
	    // ancestor branches of the project we want may also appear
	    // at this point.
	    //
	    change_completion_timestamp(cp) <= limit
	&&
	    (!detailed || !change_was_a_branch(cp))
	)
	{
	    for (j = 0; ; ++j)
	    {
		fstate_src_ty   *src;
		change_ty       *vp_cp;
		fstate_src_ty   *vp_src;
		size_t          vp_idx;

		trace(("j = %d\n", (int)j));
		src = change_file_nth(cp, j, view_path_first);
		if (!src)
		    break;
		trace(("%s\n", src->file_name->str_text));

		//
		// Add the file to the branch's state.  This is important
		// so that "looking down the view path" means something.
		//
		pbp->files->assign(src->file_name, src);

		//
		// Now look for the file along the view path.
		//
		// (Watch out, vp_idx is unsigned, so >= 0 is always true,
		// that's why its's value is one more than you expect.)
		//
		vp_cp = 0;
		vp_src = 0;
		for (vp_idx = stack.length; vp_idx > 0; --vp_idx)
		{
		    playback_ty     *vp_pbp;

		    vp_pbp = stack.item + vp_idx - 1;
		    if (vp_pbp->position >= vp_pbp->clp->length)
			continue;
		    vp_cp = vp_pbp->clp->item[vp_pbp->position];
		    vp_src =
			(fstate_src_ty *)vp_pbp->files->query(src->file_name);

		    //
		    // This logic MUST be the same as used by
		    // libaegis/project/file/list_get.c otherwise subtle
		    // and elusive bugs appear.
		    //
		    if (vp_src)
		    {
			int             found_a_concrete_file;

			found_a_concrete_file = 0;
			switch (vp_src->action)
			{
			case file_action_transparent:
			    break;

			case file_action_remove:
			    found_a_concrete_file = 1;
			    break;

			case file_action_create:
			case file_action_modify:
			case file_action_insulate:
#ifndef DEBUG
			default:
#endif
			    // should be file_action_remove
			    assert(!vp_src->deleted_by);
			    if (vp_src->deleted_by)
			    {
				found_a_concrete_file = 1;
				break;
			    }
			    // should be file_action_transparent
			    assert(!vp_src->about_to_be_created_by);
			    if (vp_src->about_to_be_created_by)
				break;
			    // should be file_action_transparent
			    assert(!vp_src->about_to_be_copied_by);
			    if (vp_src->about_to_be_copied_by)
				break;

			    found_a_concrete_file = 1;
			    break;
			}
			if (found_a_concrete_file)
			    break;
		    }

		    vp_cp = 0;
		    vp_src = 0;
		}
		assert(vp_src);
		if (!vp_src)
		    continue;

		//
		// If the file changes appearance, from the along-the-view-path
		// perspective, append the change to the list of file events.
		//
		switch (src->action)
		{
		case file_action_transparent:
		    if (idx != stack.length - 1)
			continue;
		    break;

		case file_action_create:
		case file_action_modify:
		case file_action_remove:
		case file_action_insulate:
#ifndef DEBUG
		default:
#endif
		    if (src != vp_src)
			continue;
		    break;
		}

		//
                // Locate the file events we are tracking for this file.
                // Create one if we haven't seen it before.
		//
		string_ty *uuid = uuid_or_filename(vp_src);
		file_event_list_ty *felp = uuid_to_felp.query(uuid);
		if (!felp)
		{
		    felp = file_event_list_new();
		    uuid_to_felp.assign(uuid, felp);
		    uuid_to_felp.set_reaper();
		}

		//
		// Update the mapping from file name to file UUID.
		// (Watch out for the remove half of a rename.)
		//
		if
		(
		    !vp_src->uuid
		||
		    vp_src->action != file_action_remove
		||
		    !vp_src->move
		)
		{
		    filename_to_uuid.assign(vp_src->file_name, uuid);
		}

		//
		// Note that we insert vp_cp, not cp.  This is because
		// the file entry pointed to by src may be transparent,
		// whereas the file entry pointed to by vp_src will
		// be concrete.
		//
		trace(("vp_src = %08lX\n", (long)vp_src));
		trace(("%s \"%s\" %s\n",
		    file_action_ename(vp_src->action),
		    vp_src->file_name->str_text,
		    (vp_src->edit && vp_src->edit->revision ?
		    vp_src->edit->revision->str_text : "")));
		if
		(
		    vp_src->move
		&&
		    vp_src->uuid
		&&
		    change_file_find(vp_cp, vp_src->move, view_path_first)
		)
		{
		    //
                    // Note that moved files need special handling,
                    // because they are modeled as a remove and a
                    // create.  They could come in either order, but
                    // they will both be in the same change.  Ignore the
                    // remove, but keep the create.
		    //
		    switch (vp_src->action)
		    {
		    case file_action_remove:
			break;

#ifndef DEBUG
		    default:
#endif
		    case file_action_create:
		    case file_action_modify:
		    case file_action_transparent:
		    case file_action_insulate:
			file_event_list_append
			(
			    felp,
			    playback_when(pbp),
			    vp_cp,
			    vp_src
			);
			break;
		    }
		}
		else
		{
		    file_event_list_append
		    (
			felp,
			playback_when(pbp),
			vp_cp,
			vp_src
		    );
		}
	    }
	}

	//
	// Advance this branch one change.
	//
	trace(("mark\n"));
	pbp->position++;

	//
	// Toss finished branches from the top of the stack.
	//
	while (stack.length)
	{
	    pbp = stack.item + stack.length - 1;
	    if (pbp->position < pbp->clp->length)
		break;
	    playback_list_pop(&stack);
	}
    }
    playback_list_destructor(&stack);
    trace(("return %ld;\n", (long)result));
    trace(("}\n"));
    return result;
}


project_file_roll_forward::project_file_roll_forward() :
    stp_time(0),
    last_change(0)
{
}


project_file_roll_forward::project_file_roll_forward(project_ty *pp,
	time_t limit, int detailed) :
    stp_time(0),
    last_change(0)
{
    set(pp, limit, detailed);
}


void
project_file_roll_forward::set(project_ty *pp, time_t limit, int detailed)
{
    trace(("project_file_roll_forward(pp = %08lX, limit = %ld \"%.24s\", "
	"detailed = %d)\n{\n", (long)pp, (long)limit, ctime(&limit), detailed));
    assert(filename_to_uuid.empty());
    assert(uuid_to_felp.empty());
    walk_these_branches.clear();
    stp_time = recapitulate(pp, limit, !!detailed);
    trace(("}\n"));
}


file_event_list_ty *
project_file_roll_forward::get(const nstring &filename)
{
    trace(("project_file_roll_forward_get(\"%s\")\n{\n", filename.c_str()));
    assert(!filename_to_uuid.empty());
    string_ty *uuid = filename_to_uuid.query(filename);
    if (!uuid)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    assert(!uuid_to_felp.empty());
    file_event_list_ty *result = uuid_to_felp.query(uuid);
#ifdef DEBUG
    if (result)
    {
	for (size_t j = 0; j < result->length; ++j)
	{
	    file_event_ty *fep = &result->item[j];
	    change_ty *cp = fep->cp;
	    trace(("%s, %ld, %.24s\n", project_name_get(cp->pp)->str_text,
		cp->number, ctime(&fep->when)));
	}
    }
#endif
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


file_event_list_ty *
project_file_roll_forward::get(string_ty *filename)
{
    return get(nstring(str_copy(filename)));
}


file_event_list_ty *
project_file_roll_forward::get(fstate_src_ty *src)
{
    trace(("project_file_roll_forward::get(%08lX)\n{\n", (long)src));
    trace(("src->file_name = \"%s\";\n", src->file_name->str_text));
    string_ty *uuid = uuid_or_filename(src);
    trace(("uuid = \"%s\";\n", uuid->str_text));
    assert(!uuid_to_felp.empty());
    file_event_list_ty *result = uuid_to_felp.query(uuid);
#ifdef DEBUG
    if (result)
    {
	for (size_t j = 0; j < result->length; ++j)
	{
	    file_event_ty *fep = &result->item[j];
	    change_ty *cp = fep->cp;
	    trace(("%s, %ld, %.24s\n", project_name_get(cp->pp)->str_text,
		cp->number, ctime(&fep->when)));
	}
    }
#endif
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


file_event_ty *
project_file_roll_forward::get_last(const nstring &filename)
{
    trace(("project_file_roll_forward_get_last(%s)\n{\n", filename.c_str()));
    string_ty *uuid = filename_to_uuid.query(filename);
    if (uuid == 0)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    file_event_list_ty *felp = uuid_to_felp.query(uuid);
    if (!felp || !felp->length)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }

    file_event_ty *result = &felp->item[felp->length - 1];
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


file_event_ty *
project_file_roll_forward::get_last(string_ty *filename)
{
    return get_last(nstring(str_copy(filename)));
}


file_event_ty *
project_file_roll_forward::get_older(const nstring &filename)
{
    trace(("project_file_roll_forward_get_older(%s)\n{\n", filename.c_str()));
    assert(!filename_to_uuid.empty());
    string_ty *uuid = filename_to_uuid.query(filename);
    if (!uuid)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    file_event_list_ty *felp = uuid_to_felp.query(uuid);
    if (!felp || !felp->length)
    {
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    file_event_ty *result = 0;
    for (size_t j = felp->length; j > 0; --j)
    {
	//
	// We don't simply want the second last entry, we want the
	// last entry before the time of the last change.  This is
	// because we want the previous delta, not the previous
	// file version.  For some files this will be the last entry,
	// and other files this will be the second last entry.
	//
	file_event_ty *fep = &felp->item[j - 1];
	time_t when = change_completion_timestamp(fep->cp);
	if (when < stp_time)
	{
	    result = fep;
	    break;
	}
    }
    trace(("return %08lX\n", (long)result));
    trace(("}\n"));
    return result;
}


file_event_ty *
project_file_roll_forward::get_older(string_ty *filename)
{
    return get_older(nstring(str_copy(filename)));
}


void
project_file_roll_forward::keys(nstring_list &result)
{
    assert(!filename_to_uuid.empty());
    filename_to_uuid.keys(result);
    result.sort();
}


change_ty *
project_file_roll_forward::get_last_change()
    const
{
    assert(last_change);
    return last_change;
}
