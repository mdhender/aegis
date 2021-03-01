//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate detailss
//

#include <ael/change/details.h>
#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <aer/func/now.h>
#include <change.h>
#include <change/file.h>
#include <col.h>
#include <error.h> // for assert
#include <now.h>
#include <option.h>
#include <output.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <str.h>
#include <str_list.h>
#include <trace.h>
#include <user.h>


static void
showtime(output_ty *fp, time_t when, int exempt)
{
    if (when)
    {
	struct tm *the_time = localtime(&when);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%H:%M:%S %d-%b-%Y", the_time);
	output_fputs(fp, buffer);
    }
    else if (exempt)
	output_fputs(fp, "exempt");
    else
	output_fputs(fp, "required");
}


list_change_details_columns::~list_change_details_columns()
{
    output_delete(head_col);
    head_col = 0;
    output_delete(body_col);
    body_col = 0;

    col_close(colp);
}


list_change_details_columns::list_change_details_columns()
{
    colp = col_open((string_ty *)0);

    head_col = col_create(colp, 0, 0, (const char *)0);
    body_col = col_create(colp, INDENT_WIDTH, 0, (const char *)0);
}


void
list_change_details_columns::list(change_ty *cp, bool recurse)
{
    trace(("list_change_details_columns::list()\n{\n"));
    cstate_ty *cstate_data = change_cstate_get(cp);

    //
    // Set the page heading.
    //
    if (cstate_data->branch)
    {
	if (cp->number == TRUNK_CHANGE_NUMBER)
	{
	    string_ty *line1 =
		str_format
		(
		    "Project \"%s\"",
		    project_name_get(cp->pp)->str_text
		);
	    col_title(colp, line1->str_text, "Project Details");
	    str_free(line1);
	}
	else
	{
	    string_ty *line1 =
		str_format
		(
		    "Project \"%s.%ld\"",
		    project_name_get(cp->pp)->str_text,
		    magic_zero_decode(cp->number)
		);
	    col_title(colp, line1->str_text, "Branch Details");
	    str_free(line1);
	}
    }
    else
    {
	string_ty *line1 =
	    str_format
	    (
		"Project \"%s\", Change %ld",
		project_name_get(cp->pp)->str_text,
		magic_zero_decode(cp->number)
	    );
	col_title(colp, line1->str_text, "Change Details");
	str_free(line1);
    }

    //
    // Identification.
    //
    output_fputs(head_col, "NAME");
    col_eoln(colp);
    output_fprintf
    (
	body_col,
	"Project \"%s\"",
	project_name_get(cp->pp)->str_text
    );
    if (cstate_data->delta_number)
	output_fprintf(body_col, ", Delta %ld", cstate_data->delta_number);
    if
    (
	cp->number != TRUNK_CHANGE_NUMBER
    &&
	(cstate_data->state < cstate_state_completed || option_verbose_get())
    )
    {
	output_fprintf(body_col, ", Change %ld", magic_zero_decode(cp->number));
    }
    if (cstate_data->uuid && option_verbose_get())
    {
	output_fputs(body_col, ",\n");
	output_put_str(body_col, cstate_data->uuid);
    }
    else
	output_fputc(body_col, '.');
    col_eoln(colp);

    //
    // synopsis
    //
    col_need(colp, 5);
    output_fputs(head_col, "SUMMARY");
    col_eoln(colp);
    output_put_str(body_col, cstate_data->brief_description);
    col_eoln(colp);

    //
    // description
    //
    col_need(colp, 5);
    output_fputs(head_col, "DESCRIPTION");
    col_eoln(colp);
    output_put_str(body_col, cstate_data->description);
    if (cstate_data->test_exempt || cstate_data->test_baseline_exempt)
    {
	output_end_of_line(body_col);
	output_fputc(body_col, '\n');
	if (!cstate_data->regression_test_exempt)
	{
	    output_fputs
	    (
		body_col,
		"This change must pass a full regression test.  "
	    );
	}
	if (cstate_data->test_exempt)
	{
	    output_fputs
	    (
		body_col,
                "This change is exempt from testing against the "
                "development directory. "
	    );
	}
	if (cstate_data->test_baseline_exempt)
	{
	    output_fputs
	    (
		body_col,
		"This change is exempt from testing against the baseline."
	    );
	}
    }
    col_eoln(colp);

    //
    // show the sub-changes of a branch
    //
    if (cstate_data->branch && (recurse || option_verbose_get()))
    {
	col_need(colp, 5);
	output_fputs(head_col, "BRANCH CONTENTS");
	col_eoln(colp);

	//
	// create the columns
	//
	int left = INDENT_WIDTH;
	output_ty *number_col =
	    col_create(colp, left, left + CHANGE_WIDTH, "Change\n-------");
	left += CHANGE_WIDTH + 1;
	output_ty *state_col =
	    col_create(colp, left, left + STATE_WIDTH, "State\n-------");
	left += STATE_WIDTH + 1;
	output_ty *description_col =
	    col_create(colp, left, 0, "Description\n-------------");

	//
	// list the sub changes
	//
	project_ty *sub_pp = project_bind_branch(cp->pp, cp);
	for (size_t j = 0;; ++j)
	{
	    long sub_cn;
	    if (!project_change_nth(sub_pp, j, &sub_cn))
		break;
	    change_ty *sub_cp = change_alloc(sub_pp, sub_cn);
	    change_bind_existing(sub_cp);

	    cstate_ty *sub_cstate_data = change_cstate_get(sub_cp);
	    output_fprintf(number_col, "%4ld", magic_zero_decode(sub_cn));
	    output_fputs(state_col, cstate_state_ename(sub_cstate_data->state));
	    if (cstate_data->brief_description)
	    {
		output_put_str
		(
		    description_col,
		    sub_cstate_data->brief_description
		);
	    }
	    col_eoln(colp);
	    change_free(sub_cp);
	}
	// project_free(sub_pp);
	output_delete(number_col);
	output_delete(state_col);
	output_delete(description_col);
    }
    col_eoln(colp);

    //
    // architecture
    //
    col_need(colp, 7);
    output_fprintf
    (
	head_col,
	"ARCHITECTURE%s",
	(cstate_data->architecture->length == 1 ? "" : "S")
    );
    col_eoln(colp);
    output_fputs(body_col, "This change must build and test in");
    if (cstate_data->architecture->length > 1)
	output_fputs(body_col, " each of");
    output_fputs(body_col, " the");
    for (size_t j = 0; j < cstate_data->architecture->length; ++j)
    {
	string_ty *s = cstate_data->architecture->list[j];
	if (j)
	{
	    if (j == cstate_data->architecture->length - 1)
		output_fputs(body_col, " and");
	    else
		output_fputs(body_col, ",");
	}
	output_fprintf(body_col, " \"%s\"", s->str_text);
    }
    output_fprintf
    (
	body_col,
	" architecture%s.",
	(cstate_data->architecture->length == 1 ? "" : "s")
    );
    col_eoln(colp);

    if
    (
	cstate_data->state == cstate_state_being_developed
    ||
	cstate_data->state == cstate_state_being_integrated
    )
    {
	string_list_ty	done;

	string_list_constructor(&done);
	col_need(colp, 5);
	int left = INDENT_WIDTH;
	output_ty *arch_col =
	    col_create(colp, left, left + ARCH_WIDTH, "arch.\n--------");
	left += ARCH_WIDTH + 1;
	output_ty *host_col =
	    col_create(colp, left, left + HOST_WIDTH, "host\n--------");
	left += HOST_WIDTH + 1;
	output_ty *build_col =
	    col_create(colp, left, left + TIME_WIDTH, "aeb\n---------");
	left += TIME_WIDTH + 1;
	output_ty *test_col =
	    col_create(colp, left, left + TIME_WIDTH, "aet\n---------");
	left += TIME_WIDTH + 1;
	output_ty *test_bl_col =
	    col_create(colp, left, left + TIME_WIDTH, "aet -bl\n---------");
	left += TIME_WIDTH + 1;
	output_ty *test_reg_col =
	    col_create(colp, left, left + TIME_WIDTH, "aet -reg\n---------");

	for (size_t j = 0; j < cstate_data->architecture->length; ++j)
	{
	    cstate_architecture_times_ty *tp;
	    string_ty	    *s;

	    s = cstate_data->architecture->list[j];
	    tp = change_architecture_times_find(cp, s);

	    output_put_str(arch_col, tp->variant);
	    string_list_append(&done, tp->variant);
	    if (tp->node)
		output_put_str(host_col, tp->node);
	    showtime(build_col, tp->build_time, 0);
	    showtime(test_col, tp->test_time, cstate_data->test_exempt);
	    showtime
	    (
		test_bl_col,
		tp->test_baseline_time,
		cstate_data->test_baseline_exempt
	    );
	    showtime
	    (
		test_reg_col,
		tp->regression_test_time,
		cstate_data->regression_test_exempt
	    );
	    col_eoln(colp);
	}
	for (size_t j = 0; j < cstate_data->architecture_times->length; ++j)
	{
	    cstate_architecture_times_ty *tp;

	    tp = cstate_data->architecture_times->list[j];
	    if (string_list_member(&done, tp->variant))
		continue;

	    output_put_str(arch_col, tp->variant);
	    string_list_append(&done, tp->variant);
	    if (tp->node)
		output_put_str(host_col, tp->node);
	    showtime(build_col, tp->build_time, 1);
	    showtime(test_col, tp->test_time, 1);
	    showtime(test_bl_col, tp->test_baseline_time, 1);
	    showtime(test_reg_col, tp->regression_test_time, 1);
	    col_eoln(colp);
	}
	string_list_destructor(&done);

	if (cstate_data->architecture->length > 1)
	{
	    output_fputs(build_col, "---------\n");
	    output_fputs(test_col, "---------\n");
	    output_fputs(test_bl_col, "---------\n");
	    output_fputs(test_reg_col, "---------\n");

	    showtime(build_col, cstate_data->build_time, 0);
	    showtime
	    (
		test_col,
		cstate_data->test_time,
		cstate_data->test_exempt
	    );
	    showtime
	    (
		test_bl_col,
		cstate_data->test_baseline_time,
		cstate_data->test_baseline_exempt
	    );
	    showtime
	    (
		test_reg_col,
		cstate_data->regression_test_time,
		cstate_data->regression_test_exempt
	    );
	    col_eoln(colp);
	}

	output_delete(arch_col);
	output_delete(host_col);
	output_delete(build_col);
	output_delete(test_col);
	output_delete(test_bl_col);
	output_delete(test_reg_col);
    }

    //
    // cause
    //
    col_need(colp, 5);
    output_fputs(head_col, "CAUSE");
    col_eoln(colp);
    output_fprintf
    (
	body_col,
	"This change was caused by %s.",
	change_cause_ename(cstate_data->cause)
    );
    col_eoln(colp);

    //
    // state
    //
    if (cstate_data->state != cstate_state_completed)
    {
	col_need(colp, 5);
	output_fputs(head_col, "STATE");
	col_eoln(colp);
	output_fprintf
	(
	    body_col,
	    "This change is in the '%s' state.",
	    cstate_state_ename(cstate_data->state)
	);
	col_eoln(colp);
    }

    //
    // files
    //
    col_need(colp, 5);
    output_fputs(head_col, "FILES");
    col_eoln(colp);
    if (change_file_nth(cp, (size_t)0, view_path_first))
    {
	int left = INDENT_WIDTH;
	output_ty *usage_col =
	    col_create(colp, left, left + USAGE_WIDTH, "Type\n-------");
	left += USAGE_WIDTH + 1;
	output_ty *action_col =
	    col_create(colp, left, left + ACTION_WIDTH, "Action\n--------");
	left += ACTION_WIDTH + 1;
	output_ty *edit_col =
	    col_create(colp, left, left + EDIT_WIDTH, "Edit\n-------");
	left += EDIT_WIDTH + 1;
	output_ty *file_name_col =
	    col_create(colp, left, 0, "File Name\n-----------");

	for (size_t j = 0;; ++j)
	{
	    fstate_src_ty   *src_data;

	    src_data = change_file_nth(cp, j, view_path_first);
	    if (!src_data)
		break;
	    assert(src_data->file_name);
	    output_fputs(usage_col, file_usage_ename(src_data->usage));
	    output_fputs(action_col, file_action_ename(src_data->action));
	    list_format_edit_number(edit_col, src_data);
	    if
	    (
		cstate_data->state == cstate_state_being_developed
	    &&
		!change_file_up_to_date(cp->pp, src_data)
	    )
	    {
		fstate_src_ty   *psrc_data;

		//
		// The current head revision of the
		// branch may not equal the version
		// ``originally'' copied.
		//
		psrc_data =
		    project_file_find
		    (
			cp->pp,
			src_data->file_name,
			view_path_extreme
		    );
		if (psrc_data && psrc_data->edit)
		{
		    assert(psrc_data->edit->revision);
		    output_fprintf
		    (
			edit_col,
			" (%s)",
			psrc_data->edit->revision->str_text
		    );
		}
	    }
	    if (src_data->edit_origin_new)
	    {
		//
		// The ``cross branch merge'' version.
		//
		assert(src_data->edit_origin_new->revision);
		output_end_of_line(edit_col);
		output_fprintf
		(
		    edit_col,
		    "{cross %4s}",
		    src_data->edit_origin_new->revision->str_text
		);
	    }
	    output_put_str(file_name_col, src_data->file_name);
	    if (src_data->move)
	    {
		switch (src_data->action)
		{
		case file_action_create:
		    output_end_of_line(file_name_col);
		    output_fputs(file_name_col, "Moved from ");
		    output_put_str(file_name_col, src_data->move);
		    break;

		case file_action_remove:
		    output_end_of_line(file_name_col);
		    output_fputs(file_name_col, "Moved to ");
		    output_put_str(file_name_col, src_data->move);
		    break;

		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
		    break;
		}
	    }
	    col_eoln(colp);
	}
	output_delete(usage_col);
	output_delete(action_col);
	output_delete(edit_col);
	output_delete(file_name_col);
    }
    else
    {
	output_fprintf(body_col, "This change has no files.");
	col_eoln(colp);
    }

    //
    // history
    //
    col_need(colp, 5);
    output_fputs(head_col, "HISTORY");
    col_eoln(colp);
    if (option_verbose_get())
    {
	int left = INDENT_WIDTH;
	output_ty *what_col =
	    col_create(colp, left, left + WHAT_WIDTH, "What\n------");
	left += WHAT_WIDTH + 1;
	output_ty *when_col =
	    col_create(colp, left, left + WHEN_WIDTH, "When\n------");
	left += WHEN_WIDTH + 1;
	output_ty *who_col =
	    col_create(colp, left, left + WHO_WIDTH, "Who\n-----");
	left += WHO_WIDTH + 1;
	output_ty *why_col = col_create(colp, left, 0, "Comment\n---------");

	for (size_t j = 0; j < cstate_data->history->length; ++j)
	{
	    cstate_history_ty *history_data;
	    time_t	    t;

	    history_data = cstate_data->history->list[j];
	    output_fputs
	    (
		what_col,
		cstate_history_what_ename(history_data->what)
	    );
	    t = history_data->when;
	    output_fputs(when_col, ctime(&t));
	    output_put_str(who_col, history_data->who);
	    if (history_data->why)
		output_put_str(why_col, history_data->why);
	    if (history_data->what != cstate_history_what_integrate_pass)
	    {
		time_t		finish;

		if (j + 1 < cstate_data->history->length)
		    finish = cstate_data->history->list[j + 1]->when;
		else
		    finish = now();
		if (finish - t >= ELAPSED_TIME_THRESHOLD)
		{
		    output_end_of_line(why_col);
		    output_fprintf
		    (
			why_col,
			"Elapsed time: %5.3f days.\n",
			working_days(t, finish)
		    );
		}
	    }
	    col_eoln(colp);
	}
	output_delete(what_col);
	output_delete(when_col);
	output_delete(who_col);
	output_delete(why_col);
    }
    else
    {
	if (cstate_data->state >= cstate_state_being_developed)
	{
	    output_fprintf
	    (
		body_col,
		"Developed by %s.",
		change_developer_name(cp)->str_text
	    );
	}
	if (cstate_data->state >= cstate_state_awaiting_integration)
	{
	    output_fprintf
	    (
		body_col,
		"  Reviewed by %s.",
		change_reviewer_name(cp)->str_text
	    );
	}
	if (cstate_data->state >= cstate_state_being_integrated)
	{
	    output_fprintf
	    (
		body_col,
		"  Integrated by %s.",
		change_integrator_name(cp)->str_text
	    );
	}
	col_eoln(colp);
    }

    //
    // Recurse on branches.
    //
    if (cstate_data->branch && recurse)
    {
	project_ty *sub_pp = project_bind_branch(cp->pp, cp);
	for (size_t j = 0;; ++j)
	{
	    long sub_cn;
	    if (!project_change_nth(sub_pp, j, &sub_cn))
		break;

	    change_ty *sub_cp = change_alloc(sub_pp, sub_cn);
	    change_bind_existing(sub_cp);
	    col_eject(colp);
	    list(sub_cp, true);
	    change_free(sub_cp);
	}
	// project_free(sub_pp);
    }
    trace(("}\n"));
}


void
list_change_details(string_ty *project_name, long change_number,
    string_list_ty *args)
{
    project_ty	    *pp;
    change_ty	    *cp;
    user_ty	    *up;

    //
    // locate project data
    //
    trace(("list_change_details()\n{\n"));
    if (!project_name)
	project_name = user_default_project();
    else
	project_name = str_copy(project_name);
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // List the change details.
    //
    list_change_details_columns process;
    process.list(cp, false);

    //
    // clean up and go home
    //
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}
