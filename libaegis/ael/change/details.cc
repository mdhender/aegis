//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2006, 2008 Walter Franzini
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/now.h>
#include <common/nstring.h>
#include <common/str.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/details.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/ael/formeditnum.h>
#include <libaegis/aer/func/now.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


static void
showtime(output::pointer fp, time_t when, bool exempt)
{
    if (when)
    {
        struct tm *the_time = localtime(&when);
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%H:%M:%S %d-%b-%Y", the_time);
        fp->fputs(buffer);
    }
    else if (exempt)
        fp->fputs("exempt");
    else
        fp->fputs("required");
}


list_change_details_columns::~list_change_details_columns()
{
}


list_change_details_columns::list_change_details_columns() :
    colp(col::open((string_ty *)0)),
    head_col(colp->create(0, 0, (const char *)0)),
    body_col(colp->create(INDENT_WIDTH, 0, (const char *)0))
{
}


void
list_change_details_columns::list(change::pointer cp, bool recurse)
{
    trace(("list_change_details_columns::list()\n{\n"));
    cstate_ty *cstate_data = cp->cstate_get();

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
                    project_name_get(cp->pp).c_str()
                );
            colp->title(line1->str_text, "Project Details");
            str_free(line1);
        }
        else
        {
            string_ty *line1 =
                str_format
                (
                    "Project \"%s.%ld\"",
                    project_name_get(cp->pp).c_str(),
                    magic_zero_decode(cp->number)
                );
            colp->title(line1->str_text, "Branch Details");
            str_free(line1);
        }
    }
    else
    {
        string_ty *line1 =
            str_format
            (
                "Project \"%s\", Change %ld",
                project_name_get(cp->pp).c_str(),
                magic_zero_decode(cp->number)
            );
        colp->title(line1->str_text, "Change Details");
        str_free(line1);
    }

    //
    // Identification.
    //
    head_col->fputs("NAME");
    colp->eoln();
    body_col->fprintf("Project \"%s\"", project_name_get(cp->pp).c_str());
    if (cstate_data->delta_number)
        body_col->fprintf(", Delta %ld", cstate_data->delta_number);
    if
    (
        cp->number != TRUNK_CHANGE_NUMBER
    &&
        (cstate_data->state < cstate_state_completed || option_verbose_get())
    )
    {
        body_col->fprintf(", Change %ld", magic_zero_decode(cp->number));
    }
    if (cstate_data->uuid && option_verbose_get())
    {
        body_col->fputs(",\n");
        body_col->fputs(cstate_data->uuid);
    }
    else
        body_col->fputc('.');
    colp->eoln();

    //
    // synopsis
    //
    colp->need(5);
    head_col->fputs("SUMMARY");
    colp->eoln();
    body_col->fputs(cstate_data->brief_description);
    colp->eoln();

    //
    // description
    //
    colp->need(5);
    head_col->fputs("DESCRIPTION");
    colp->eoln();
    body_col->fputs(cstate_data->description);
    if (cstate_data->test_exempt || cstate_data->test_baseline_exempt)
    {
        body_col->end_of_line();
        body_col->fputc('\n');
        if (!cstate_data->regression_test_exempt)
        {
            body_col->fputs("This change must pass a full regression test.  ");
        }
        if (cstate_data->test_exempt)
        {
            body_col->fputs
            (
                "This change is exempt from testing against the "
                "development directory. "
            );
        }
        if (cstate_data->test_baseline_exempt)
        {
            body_col->fputs
            (
                "This change is exempt from testing against the baseline."
            );
        }
    }
    colp->eoln();

    //
    // List attributes
    //
    if (cstate_data->attribute && cstate_data->attribute->length)
    {
        colp->need(5);
        head_col->fputs("ATTRIBUTES");
        colp->eoln();

        //
        // create the columns
        //
        int left = INDENT_WIDTH;
        output::pointer name_col =
            colp->create(left, left + ATTR_WIDTH, "Name\n-------");
        left += ATTR_WIDTH + 1;
        output::pointer value_col =
            colp->create(left, 0, "Value\n-------");

        //
        // list the attributes
        //
        attributes_list_ty *alp = cstate_data->attribute;
        for (size_t j = 0; j < alp->length; ++j)
        {
            attributes_ty *ap = alp->list[j];
            if (ap->name)
                name_col->fputs(ap->name);
            if (ap->value)
                value_col->fputs(ap->value);
            colp->eoln();
        }

        colp->forget(name_col);
        colp->forget(value_col);
    }

    //
    // show the sub-changes of a branch
    //
    if (cstate_data->branch && (recurse || option_verbose_get()))
    {
        colp->need(5);
        head_col->fputs("BRANCH CONTENTS");
        colp->eoln();

        //
        // create the columns
        //
        int left = INDENT_WIDTH;
        output::pointer number_col =
            colp->create(left, left + CHANGE_WIDTH, "Change\n-------");
        left += CHANGE_WIDTH + 1;
        output::pointer state_col =
            colp->create(left, left + STATE_WIDTH, "State\n-------");
        left += STATE_WIDTH + 1;
        output::pointer description_col =
            colp->create(left, 0, "Description\n-------------");

        //
        // list the sub changes
        //
        project *sub_pp = cp->pp->bind_branch(cp);
        for (size_t j = 0;; ++j)
        {
            long sub_cn;
            if (!project_change_nth(sub_pp, j, &sub_cn))
                break;
            change::pointer sub_cp = change_alloc(sub_pp, sub_cn);
            change_bind_existing(sub_cp);

            cstate_ty *sub_cstate_data = sub_cp->cstate_get();
            number_col->fprintf("%4ld", magic_zero_decode(sub_cn));
            state_col->fputs(cstate_state_ename(sub_cstate_data->state));
            if (cstate_data->brief_description)
            {
                description_col->fputs(sub_cstate_data->brief_description);
            }
            colp->eoln();
            change_free(sub_cp);
        }

        colp->forget(number_col);
        colp->forget(state_col);
        colp->forget(description_col);
    }
    colp->eoln();

    //
    // architecture
    //
    colp->need(7);
    head_col->fprintf
    (
        "ARCHITECTURE%s",
        (cstate_data->architecture->length == 1 ? "" : "S")
    );
    colp->eoln();
    body_col->fputs("This change must ");
    bool build_required = change_build_required(cp, !cp->was_a_branch());
    if (build_required)
        body_col->fputs("build and ");
    body_col->fputs("test in");
    if (cstate_data->architecture->length > 1)
        body_col->fputs(" each of");
    body_col->fputs(" the");
    for (size_t k = 0; k < cstate_data->architecture->length; ++k)
    {
        string_ty *s = cstate_data->architecture->list[k];
        if (k)
        {
            if (k == cstate_data->architecture->length - 1)
                body_col->fputs(" and");
            else
                body_col->fputc(',');
        }
        body_col->fprintf(" \"%s\"", s->str_text);
    }
    body_col->fprintf
    (
        " architecture%s.",
        (cstate_data->architecture->length == 1 ? "" : "s")
    );
    colp->eoln();

    if
    (
        cstate_data->state == cstate_state_being_developed
    ||
        cstate_data->state == cstate_state_being_integrated
    )
    {
        string_list_ty  done;
        colp->need(5);
        int left = INDENT_WIDTH;
        output::pointer arch_col =
            colp->create(left, left + ARCH_WIDTH, "arch.\n--------");
        left += ARCH_WIDTH + 1;
        output::pointer host_col =
            colp->create(left, left + HOST_WIDTH, "host\n--------");
        left += HOST_WIDTH + 1;
        output::pointer build_col =
            colp->create(left, left + TIME_WIDTH, "aeb\n---------");
        left += TIME_WIDTH + 1;
        output::pointer test_col =
            colp->create(left, left + TIME_WIDTH, "aet\n---------");
        left += TIME_WIDTH + 1;
        output::pointer test_bl_col =
            colp->create(left, left + TIME_WIDTH, "aet -bl\n---------");
        left += TIME_WIDTH + 1;
        output::pointer test_reg_col =
            colp->create(left, left + TIME_WIDTH, "aet -reg\n---------");

        for (size_t j = 0; j < cstate_data->architecture->length; ++j)
        {
            cstate_architecture_times_ty *tp;
            string_ty       *s;

            s = cstate_data->architecture->list[j];
            tp = change_architecture_times_find(cp, s);

            arch_col->fputs(tp->variant);
            done.push_back(tp->variant);
            if (tp->node)
                host_col->fputs(tp->node);
            showtime(build_col, tp->build_time, !build_required);
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
            colp->eoln();
        }
        for (size_t k = 0; k < cstate_data->architecture_times->length; ++k)
        {
            cstate_architecture_times_ty *tp =
                cstate_data->architecture_times->list[k];
            if (done.member(tp->variant))
                continue;

            arch_col->fputs(tp->variant);
            done.push_back(tp->variant);
            if (tp->node)
                host_col->fputs(tp->node);
            showtime(build_col, tp->build_time, true);
            showtime(test_col, tp->test_time, true);
            showtime(test_bl_col, tp->test_baseline_time, true);
            showtime(test_reg_col, tp->regression_test_time, true);
            colp->eoln();
        }

        if (cstate_data->architecture->length > 1)
        {
            build_col->fputs("---------\n");
            test_col->fputs("---------\n");
            test_bl_col->fputs("---------\n");
            test_reg_col->fputs("---------\n");

            showtime
            (
                build_col,
                cstate_data->build_time,
                !build_required
            );
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
            colp->eoln();
        }

        colp->forget(arch_col);
        colp->forget(host_col);
        colp->forget(build_col);
        colp->forget(test_col);
        colp->forget(test_bl_col);
        colp->forget(test_reg_col);
    }

    //
    // cause
    //
    colp->need(5);
    head_col->fputs("CAUSE");
    colp->eoln();
    body_col->fprintf
    (
        "This change was caused by %s.",
        change_cause_ename(cstate_data->cause)
    );
    colp->eoln();

    //
    // state
    //
    if (cstate_data->state != cstate_state_completed)
    {
        colp->need(5);
        head_col->fputs("STATE");
        colp->eoln();
        body_col->fprintf
        (
            "This change is in the '%s' state.",
            cstate_state_ename(cstate_data->state)
        );
        colp->eoln();
    }

    //
    // files
    //
    colp->need(5);
    head_col->fputs("FILES");
    colp->eoln();
    if (change_file_nth(cp, (size_t)0, view_path_first))
    {
        int left = INDENT_WIDTH;
        output::pointer usage_col =
            colp->create(left, left + USAGE_WIDTH, "Type\n-------");
        left += USAGE_WIDTH + 1;
        output::pointer action_col =
            colp->create(left, left + ACTION_WIDTH, "Action\n--------");
        left += ACTION_WIDTH + 1;
        output::pointer edit_col =
            colp->create(left, left + EDIT_WIDTH, "Edit\n-------");
        left += EDIT_WIDTH + 1;
        output::pointer file_name_col =
            colp->create(left, 0, "File Name\n-----------");

        for (size_t j = 0;; ++j)
        {
            fstate_src_ty   *src_data;

            src_data = change_file_nth(cp, j, view_path_first);
            if (!src_data)
                break;
            assert(src_data->file_name);
            usage_col->fputs(file_usage_ename(src_data->usage));
            action_col->fputs(file_action_ename(src_data->action));
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
                // The current head revision of the branch may not equal
                // the version "originally" copied.
                //
                psrc_data = cp->pp->file_find(src_data, view_path_extreme);
                if (psrc_data && psrc_data->edit)
                {
                    assert(psrc_data->edit->revision);
                    edit_col->fprintf
                    (
                        " (%s)",
                        psrc_data->edit->revision->str_text
                    );
                }
            }
            if (src_data->edit_origin_new)
            {
                //
                // The "cross branch merge" version.
                //
                assert(src_data->edit_origin_new->revision);
                edit_col->end_of_line();
                edit_col->fprintf
                (
                    "{cross %4s}",
                    src_data->edit_origin_new->revision->str_text
                );
            }
            file_name_col->fputs(src_data->file_name);
            if (src_data->move)
            {
                switch (src_data->action)
                {
                case file_action_create:
                    file_name_col->end_of_line();
                    file_name_col->fputs("Moved from ");
                    file_name_col->fputs(src_data->move);
                    break;

                case file_action_remove:
                    file_name_col->end_of_line();
                    file_name_col->fputs("Moved to ");
                    file_name_col->fputs(src_data->move);
                    break;

                case file_action_modify:
                case file_action_insulate:
                case file_action_transparent:
                    break;
                }
            }
            colp->eoln();
        }
        colp->forget(usage_col);
        colp->forget(action_col);
        colp->forget(edit_col);
        colp->forget(file_name_col);
    }
    else
    {
        body_col->fputs("This change has no files.");
        colp->eoln();
    }

    //
    // history
    //
    colp->need(5);
    head_col->fputs("HISTORY");
    colp->eoln();
    if (option_verbose_get())
    {
        int left = INDENT_WIDTH;
        output::pointer what_col =
            colp->create(left, left + WHAT_WIDTH, "What\n------");
        left += WHAT_WIDTH + 1;
        output::pointer when_col =
            colp->create(left, left + WHEN_WIDTH, "When\n------");
        left += WHEN_WIDTH + 1;
        output::pointer who_col =
            colp->create(left, left + WHO_WIDTH, "Who\n-----");
        left += WHO_WIDTH + 1;
        output::pointer why_col = colp->create(left, 0, "Comment\n---------");
        output::pointer wide_why_col = colp->create(12, 0, 0);

        for (size_t j = 0; j < cstate_data->history->length; ++j)
        {
            cstate_history_ty *history_data;
            time_t          t;

            history_data = cstate_data->history->list[j];
            what_col->fputs(cstate_history_what_ename(history_data->what));
            t = history_data->when;
            when_col->fputs(ctime(&t));
            who_col->fputs(history_data->who);
            if (history_data->why && history_data->why->str_length <= 40)
                why_col->fputs(history_data->why);
            if (history_data->what != cstate_history_what_integrate_pass)
            {
                time_t          finish;

                if (j + 1 < cstate_data->history->length)
                    finish = cstate_data->history->list[j + 1]->when;
                else
                    finish = now();
                if (finish - t >= ELAPSED_TIME_THRESHOLD)
                {
                    why_col->end_of_line();
                    why_col->fprintf
                    (
                        "Elapsed time: %5.3f days.\n",
                        working_days(t, finish)
                    );
                }
            }
            colp->eoln();
            if (history_data->why && history_data->why->str_length > 40)
            {
                wide_why_col->fputs(history_data->why);
                colp->eoln();
            }
        }
        colp->forget(what_col);
        colp->forget(when_col);
        colp->forget(who_col);
        colp->forget(why_col);
        colp->forget(wide_why_col);
    }
    else
    {
        if (cstate_data->state >= cstate_state_being_developed)
        {
            body_col->fprintf
            (
                "Developed by %s.",
                cp->developer_name()->str_text
            );
        }
        if (cstate_data->state >= cstate_state_awaiting_integration)
        {
            body_col->fprintf
            (
                "  Reviewed by %s.",
                cp->reviewer_name()->str_text
            );
        }
        if (cstate_data->state >= cstate_state_being_integrated)
        {
            body_col->fprintf
            (
                "  Integrated by %s.",
                cp->integrator_name()->str_text
            );
        }
        colp->eoln();
    }

    //
    // Recurse on branches.
    //
    if (cstate_data->branch && recurse)
    {
        project *sub_pp = cp->pp->bind_branch(cp);
        for (size_t j = 0;; ++j)
        {
            long sub_cn;
            if (!project_change_nth(sub_pp, j, &sub_cn))
                break;

            change::pointer sub_cp = change_alloc(sub_pp, sub_cn);
            change_bind_existing(sub_cp);
            colp->eject();
            list(sub_cp, true);
            change_free(sub_cp);
        }
    }
    trace(("}\n"));
}


void
list_change_details(change_identifier &cid, string_list_ty *)
{
    trace(("list_change_details()\n{\n"));
    list_change_details_columns process;
    process.list(cid.get_cp(), false);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
