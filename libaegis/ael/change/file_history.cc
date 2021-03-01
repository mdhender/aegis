//
//	aegis - project change supervisor
//	Copyright (C) 2001-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/error.h> // for assert
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/file_history.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/col.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/user.h>


#define VERSION_WIDTH 10


static bool
is_a_rename_with_uuid(change::pointer cp, fstate_src_ty *src)
{
    if (src->action != file_action_create)
	return false;
    if (!src->move)
	return false;
    if (!src->uuid)
	return false;
    fstate_src_ty *other = change_file_find(cp, src->move, view_path_first);
    if (!other)
	return false;
    if (!other->uuid)
	return false;
    return str_equal(src->uuid, other->uuid);
}


void
list_change_file_history(string_ty *project_name, long change_number,
    string_list_ty *)
{
    project_ty	    *pp;
    change::pointer cp;
    user_ty::pointer up;
    output_ty	    *usage_col;
    output_ty	    *action_col;
    output_ty	    *when_col;
    output_ty	    *file_name_col;
    int		    j;
    string_ty	    *line1;
    int		    left;
    col	    *colp;
    output_ty	    *delta_col;
    output_ty	    *change_col;
    output_ty	    *description_col;

    //
    // locate project data
    //
    trace(("list_change_file_history()\n{\n"));
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    assert(change_file_nth(cp, 0, view_path_none));

    //
    // Reconstruct the project file history.
    //
    time_t when = change_completion_timestamp(cp);
    project_file_roll_forward historian(pp, when, option_verbose_get());

    //
    // create the columns
    //
    colp = col::open((string_ty *)0);
    line1 =
	str_format
	(
	    "Project \"%s\"  Change %ld",
	    project_name_get(pp)->str_text,
	    magic_zero_decode(change_number)
	);
    colp->title(line1->str_text, "Change File History");
    str_free(line1);

    file_name_col = colp->create(0, 0, "File Name\n-----------");

    left = 2;
    usage_col = colp->create(left, left + USAGE_WIDTH, 0);
    left += USAGE_WIDTH + 1;
    action_col = colp->create(left, left + ACTION_WIDTH, 0);
    left += ACTION_WIDTH + 1;

    delta_col =
	colp->create(left, left + VERSION_WIDTH, "Delta\n---------");
    left += VERSION_WIDTH + 1;

    when_col =
	colp->create(left, left + WHEN_WIDTH, "Date and Time\n---------------");
    left += WHEN_WIDTH + 1;

    change_col = colp->create(left, left + CHANGE_WIDTH, "Change\n-------");
    left += CHANGE_WIDTH + 1;

    description_col = colp->create(left, 0, "Description\n-------------");

    //
    // list the change's files' histories
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty	*src_data;
	file_event_list::pointer felp;
	size_t		k;
	int		usage_track;
	int		action_track;

	src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;
	assert(src_data->file_name);
	colp->need(4);

	string_ty *file_name_track = 0;
	usage_track = -1;
	action_track = -1;

	felp = historian.get(src_data);
	if (felp)
	{
	    for (k = 0; k < felp->size(); ++k)
	    {
		file_event	*fep;
		string_ty	*s;

		fep = felp->get(k);
		assert(fep->get_src());

		//
		// See if the file's name changed.
		//
		if (!str_equal(file_name_track, fep->get_src()->file_name))
		{
		    file_name_col->fputs(fep->get_src()->file_name->str_text);
		    colp->eoln();
		    file_name_track = fep->get_src()->file_name;
		}

		s = change_version_get(fep->get_change());
		delta_col->fputs(s->str_text);
		str_free(s);
		change_col->fprintf("%4ld", fep->get_change()->number);

		if (usage_track != fep->get_src()->usage)
		{
		    usage_col->fputs(file_usage_ename(fep->get_src()->usage));
		    usage_track = fep->get_src()->usage;
		    action_track = -1;
		}
		if (action_track != fep->get_src()->action)
		{
		    if
		    (
			is_a_rename_with_uuid(fep->get_change(), fep->get_src())
		    )
		    {
			action_col->fputs("rename");
			action_track = -1;
		    }
		    else
		    {
			action_col->fputs
			(
			    file_action_ename(fep->get_src()->action)
			);
			action_track = fep->get_src()->action;
		    }
		}
		time_t when3 = fep->get_when();
		when_col->fputs(ctime(&when3));
		description_col->fputs
		(
		    change_brief_description_get(fep->get_change())->str_text
		);
		colp->eoln();
	    }
	}

	//
	// Now output details of this change, as the "end"
	// of the history.
	//
	if (!cp->is_completed())
	{
	    //
	    // See if the file's name changed.
	    //
	    if (!str_equal(file_name_track, src_data->file_name))
	    {
		file_name_col->fputs(src_data->file_name->str_text);
		colp->eoln();
	    }

	    delta_col->fputs(change_version_get(cp)->str_text);
	    change_col->fprintf("%4ld", cp->number);
	    if (usage_track != src_data->usage)
	    {
		usage_col->fputs(file_usage_ename(src_data->usage));
		action_track = -1;
	    }
	    if (action_track != src_data->action)
	    {
		if (is_a_rename_with_uuid(cp, src_data))
		{
		    action_col->fputs("rename");
		}
		else
		{
		    action_col->fputs(file_action_ename(src_data->action));
		}
	    }
	    description_col->fputs(change_brief_description_get(cp)->str_text);
	    colp->eoln();
	}
    }

    //
    // clean up and go home
    //
    delete colp;
    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}
