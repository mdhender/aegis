/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate detailss
 */

#include <ac/time.h>

#include <ael/change/details.h>
#include <ael/column_width.h>
#include <ael/formeditnum.h>
#include <aer/func/now.h>
#include <change.h>
#include <change/file.h>
#include <col.h>
#include <error.h> /* for assert */
#include <option.h>
#include <project.h>
#include <project/file.h>
#include <project_hist.h>
#include <str.h>
#include <trace.h>
#include <user.h>


static void showtime _((int, time_t, int));

static void
showtime(colnum, when, exempt)
	int	colnum;
	time_t	when;
	int	exempt;
{
	if (when)
	{
		struct tm	*tm;
		char		buffer[100];

		tm = localtime(&when);
		strftime(buffer, sizeof(buffer), "%H:%M:%S %d-%b-%y", tm);
		col_puts(colnum, buffer);
	}
	else if (exempt)
		col_puts(colnum, "exempt");
	else
		col_puts(colnum, "required");
}


void
list_change_details(project_name, change_number)
	string_ty	*project_name;
	long		change_number;
{
	int		j;
	int		head_col;
	int		body_col;
	project_ty	*pp;
	change_ty	*cp;
	cstate		cstate_data;
	user_ty		*up;
	string_ty	*line1;
	int		left;

	/*
	 * locate project data
	 */
	trace(("list_change_details()\n{\n"));
	if (!project_name)
		project_name = user_default_project();
	else
		project_name = str_copy(project_name);
	pp = project_alloc(project_name);
	str_free(project_name);
	project_bind_existing(pp);

	/*
	 * locate user data
	 */
	up = user_executing(pp);

	/*
	 * locate change data
	 */
	if (!change_number)
		change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_data = change_cstate_get(cp);

	/*
	 * identification
	 */
	col_open((char *)0);
	line1 =
		str_format
		(
			"Project \"%S\", Change %ld",
			project_name_get(pp),
			magic_zero_decode(change_number)
		);
	col_title(line1->str_text, "Change Details");
	str_free(line1);

	/* the heading columns is the whole page wide */
	head_col = col_create(0, 0);

	/* the body columns is indented */
	body_col = col_create(INDENT_WIDTH, 0);
	col_puts(head_col, "NAME");
	col_eoln();
	col_printf(body_col, "Project \"%s\"", project_name_get(pp)->str_text);
	if (cstate_data->delta_number)
		col_printf(body_col, ", Delta %ld", cstate_data->delta_number);
	if (cstate_data->state < cstate_state_completed || option_verbose_get())
	{
		col_printf
		(
			body_col,
			", Change %ld",
			magic_zero_decode(change_number)
		);
	}
	col_puts(body_col, ".");
	col_eoln();

	/*
	 * synopsis
	 */
	col_need(5);
	col_puts(head_col, "SUMMARY");
	col_eoln();
	col_puts(body_col, cstate_data->brief_description->str_text);
	col_eoln();

	/*
	 * description
	 */
	col_need(5);
	col_puts(head_col, "DESCRIPTION");
	col_eoln();
	col_puts(body_col, cstate_data->description->str_text);
	if (cstate_data->test_exempt || cstate_data->test_baseline_exempt)
	{
		col_bol(body_col);
		col_puts(body_col, "\n");
		if (!cstate_data->regression_test_exempt)
		{
			col_puts
			(
				body_col,
			       "This change must pass a full regression test.  "
			);
		}
		if (cstate_data->test_exempt)
		{
			col_puts
			(
				body_col,
       "This change is exempt from testing against the development directory.  "
			);
		}
		if (cstate_data->test_baseline_exempt)
		{
			col_puts
			(
				body_col,
		      "This change is exempt from testing against the baseline."
			);
		}
	}
	col_eoln();

	/*
	 * show the sub-changes of a branch
	 */
	if (cstate_data->branch && option_verbose_get())
	{
		project_ty	*sub_pp;
		int		number_col;
		int		state_col;
		int		description_col;

		col_need(5);
		col_puts(head_col, "BRANCH CONTENTS");
		col_eoln();

		/*
		 * create the columns
		 */
		left = INDENT_WIDTH;
		number_col = col_create(left, left + CHANGE_WIDTH);
		left += CHANGE_WIDTH + 1;
		col_heading(number_col, "Change\n-------");

		state_col = col_create(left, left + STATE_WIDTH);
		left += STATE_WIDTH + 1;
		col_heading(state_col, "State\n-------");

		description_col = col_create(left, 0);
		col_heading(description_col, "Description\n-------------");

		/*
		 * list the sub changes
		 */
		sub_pp = project_bind_branch(pp, cp);
		for (j = 0; ; ++j)
		{
			long		sub_cn;
			change_ty	*sub_cp;
			cstate		sub_cstate_data;

			if (!project_change_nth(sub_pp, j, &sub_cn))
				break;
			sub_cp = change_alloc(sub_pp, sub_cn);
			change_bind_existing(sub_cp);

			sub_cstate_data = change_cstate_get(sub_cp);
			col_printf
			(
				number_col,
				"%4ld",
				magic_zero_decode(sub_cn)
			);
			col_puts
			(
				state_col,
				cstate_state_ename(sub_cstate_data->state)
			);
			if (cstate_data->brief_description)
			{
				col_puts
				(
					description_col,
					sub_cstate_data->brief_description->str_text
				);
			}
			col_eoln();
			change_free(sub_cp);
		}
		col_heading(number_col, (char *)0);
		col_heading(state_col, (char *)0);
		col_heading(description_col, (char *)0);
	}
	col_eoln();

	/*
	 * architecture
	 */
	col_need(7);
	col_printf
	(
		head_col,
		"ARCHITECTURE%s",
		(cstate_data->architecture->length == 1 ? "" : "S")
	);
	col_eoln();
	col_puts(body_col, "This change must build and test in");
	if (cstate_data->architecture->length > 1)
		col_puts(body_col, " each of");
	col_puts(body_col, " the");
	for (j = 0; j < cstate_data->architecture->length; ++j)
	{
		string_ty	*s;

		s = cstate_data->architecture->list[j];
		if (j)
		{
			if (j == cstate_data->architecture->length - 1)
				col_puts(body_col, " and");
			else
				col_puts(body_col, ",");
		}
		col_printf(body_col, " \"%s\"",  s->str_text);
	}
	col_printf
	(
		body_col,
		" architecture%s.",
		(cstate_data->architecture->length == 1 ? "" : "s")
	);
	col_eoln();

	if
	(
		cstate_data->state == cstate_state_being_developed
	||
		cstate_data->state == cstate_state_being_integrated
	)
	{
		int	arch_col;
		int	host_col;
		int	build_col;
		int	test_col;
		int	test_bl_col;
		int	test_reg_col;

		col_need(5);
		left = INDENT_WIDTH;
		arch_col = col_create(left, left + ARCH_WIDTH);
		left += ARCH_WIDTH + 1;
		col_heading(arch_col, "arch.\n--------");

		host_col = col_create(left, left + HOST_WIDTH);
		left += HOST_WIDTH + 1;
		col_heading(host_col, "host\n--------");

		build_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(build_col, "aeb\n---------");

		test_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(test_col, "aet\n---------");

		test_bl_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(test_bl_col, "aet -bl\n---------");

		test_reg_col = col_create(left, left + TIME_WIDTH);
		left += TIME_WIDTH + 1;
		col_heading(test_reg_col, "aet -reg\n---------");

		for (j = 0; j < cstate_data->architecture->length; ++j)
		{
			cstate_architecture_times tp;
			string_ty	*s;

			s = cstate_data->architecture->list[j];
			tp = change_architecture_times_find(cp, s);
			
			col_puts(arch_col, tp->variant->str_text);
			if (tp->node)
				col_puts(host_col, tp->node->str_text);
			showtime(build_col, tp->build_time, 0);
			showtime
			(
				test_col,
				tp->test_time,
				cstate_data->test_exempt
			);
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
			col_eoln();
		}

		col_heading(arch_col, (char *)0);
		col_heading(host_col, (char *)0);
		col_heading(build_col, (char *)0);
		col_heading(test_col, (char *)0);
		col_heading(test_bl_col, (char *)0);
		col_heading(test_reg_col, (char *)0);

		if (cstate_data->architecture->length > 1)
		{
			col_puts(build_col, "---------\n");
			col_puts(test_col, "---------\n");
			col_puts(test_bl_col, "---------\n");
			col_puts(test_reg_col, "---------\n");

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
			col_eoln();
		}
	}

	/*
	 * cause
	 */
	col_need(5);
	col_puts(head_col, "CAUSE");
	col_eoln();
	col_printf
	(
		body_col,
		"This change was caused by %s.",
		change_cause_ename(cstate_data->cause)
	);
	col_eoln();

	/*
	 * state
	 */
	if (cstate_data->state != cstate_state_completed)
	{
		col_need(5);
		col_puts(head_col, "STATE");
		col_eoln();
		col_printf
		(
			body_col,
			"This change is in the '%s' state.",
			cstate_state_ename(cstate_data->state)
		);
		col_eoln();
	}

	/*
	 * files
	 */
	col_need(5);
	col_puts(head_col, "FILES");
	col_eoln();
	if (change_file_nth(cp, (size_t)0))
	{
		int	usage_col;
		int	action_col;
		int	edit_col;
		int	file_name_col;

		left = INDENT_WIDTH;
		usage_col = col_create(left, left + USAGE_WIDTH);
		left += USAGE_WIDTH + 1;
		col_heading(usage_col, "Type\n-------");

		action_col = col_create(left, left + ACTION_WIDTH);
		left += ACTION_WIDTH + 1;
		col_heading(action_col, "Action\n--------");

		edit_col = col_create(left, left + EDIT_WIDTH);
		left += EDIT_WIDTH + 1;
		col_heading(edit_col, "Edit\n-------");

		file_name_col = col_create(left, 0);
		col_heading(file_name_col, "File Name\n-----------");
		for (j = 0; ; ++j)
		{
			fstate_src	src_data;
	
			src_data = change_file_nth(cp, j);
			if (!src_data)
				break;
			assert(src_data->file_name);
			col_puts(usage_col, file_usage_ename(src_data->usage));
			col_puts
			(
				action_col,
				file_action_ename(src_data->action)
			);
			list_format_edit_number(edit_col, src_data);
			if
			(
			      cstate_data->state == cstate_state_being_developed
			&&
				!change_file_up_to_date(pp, src_data)
			)
			{
				fstate_src	psrc_data;

				/*
				 * The current head revision of the
				 * branch may not equal the version
				 * ``originally'' copied.
				 */
				psrc_data =
					project_file_find
					(
						pp,
						src_data->file_name
					);
				if (psrc_data && psrc_data->edit_number)
				{
					col_printf
					(
						edit_col,
						" (%s)",
						psrc_data->edit_number->str_text
					);
				}
			}
			if (src_data->edit_number_origin_new)
			{
				/*
				 * The ``cross branch merge'' version.
				 */
				col_bol(edit_col);
				col_printf
				(
					edit_col,
					"{cross %4s}",
				      src_data->edit_number_origin_new->str_text
				);
			}
			col_puts(file_name_col, src_data->file_name->str_text);
			if (src_data->move)
			{
				col_bol(file_name_col);
				col_puts(file_name_col, "Moved ");
				if (src_data->action == file_action_create)
					col_puts(file_name_col, "from ");
				else
					col_puts(file_name_col, "to ");
				col_puts
				(
					file_name_col,
					src_data->move->str_text
				);
			}
			col_eoln();
		}
		col_heading(usage_col, (char *)0);
		col_heading(action_col, (char *)0);
		col_heading(edit_col, (char *)0);
		col_heading(file_name_col, (char *)0);
	}
	else
	{
		col_printf(body_col, "This change has no files.");
		col_eoln();
	}

	/*
	 * history
	 */
	col_need(5);
	col_puts(head_col, "HISTORY");
	col_eoln();
	if (option_verbose_get())
	{
		int	what_col;
		int	when_col;
		int	who_col;
		int	why_col;

		left = INDENT_WIDTH;
		what_col = col_create(left, left + WHAT_WIDTH);
		left += WHAT_WIDTH + 1;
		col_heading(what_col, "What\n------");

		when_col = col_create(left, left + WHEN_WIDTH);
		left += WHEN_WIDTH + 1;
		col_heading(when_col, "When\n------");

		who_col = col_create(left, left + WHO_WIDTH);
		left += WHO_WIDTH + 1;
		col_heading(who_col, "Who\n-----");

		why_col = col_create(left, 0);
		col_heading(why_col, "Comment\n---------");
		for (j = 0; j < cstate_data->history->length; ++j)
		{
			cstate_history	history_data;
			time_t		t;

			history_data = cstate_data->history->list[j];
			col_puts
			(
				what_col,
				cstate_history_what_ename(history_data->what)
			);
			t = history_data->when;
			col_puts(when_col, ctime(&t));
			col_puts(who_col, history_data->who->str_text);
			if (history_data->why)
				col_puts(why_col, history_data->why->str_text);
			if
			(
				history_data->what
			!=
				cstate_history_what_integrate_pass
			)
			{
				time_t	finish;

				if (j + 1 < cstate_data->history->length)
					finish =
						cstate_data->history->
							list[j + 1]->
								when;
				else
					time(&finish);
				if (finish - t >= ELAPSED_TIME_THRESHOLD)
				{
					col_bol(why_col);
					col_printf
					(
						why_col,
						"Elapsed time: %5.3f days.\n",
						working_days(t, finish)
					);
				}
			}
			col_eoln();
		}
		col_heading(what_col, (char *)0);
		col_heading(when_col, (char *)0);
		col_heading(who_col, (char *)0);
		col_heading(why_col, (char *)0);
	}
	else
	{
		if (cstate_data->state >= cstate_state_being_developed)
		{
			col_printf
			(
				body_col,
				"Developed by %s.",
				change_developer_name(cp)->str_text
			);
		}
		if (cstate_data->state >= cstate_state_awaiting_integration)
		{
			col_printf
			(
				body_col,
				"  Reviewed by %s.",
				change_reviewer_name(cp)->str_text
			);
		}
		if (cstate_data->state >= cstate_state_being_integrated)
		{
			col_printf
			(
				body_col,
				"  Integrated by %s.",
				change_integrator_name(cp)->str_text
			);
		}
		col_eoln();
	}

	/*
	 * clean up and go home
	 */
	col_close();
	change_free(cp);
	project_free(pp);
	user_free(up);
	trace(("}\n"));
}
