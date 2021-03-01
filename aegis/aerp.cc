//
// aegis - project change supervisor
// Copyright (C) 1991-1999, 2001-2009, 2011, 2012 Peter Miller
// Copyright (C) 2008 Walter Franzini
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
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/mem.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change/run/review_polic.h>
#include <libaegis/change/signedoffby.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/pattr.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aerp.h>


static void
review_pass_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Review_PASS <change_number> [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Review_PASS -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Review_PASS -Help\n", progname);
    quit(1);
}


static void
review_pass_help(void)
{
    help("aerpass", review_pass_usage);
}


static void
review_pass_list(void)
{
    trace(("review_pass_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(review_pass_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_reviewed);
    trace(("}\n"));
}


static void
check_permissions(change::pointer cp, user_ty::pointer up)
{
    cstate_ty       *cstate_data;
    project      *pp;
    cstate_history_ty *hp;

    cstate_data = cp->cstate_get();

    //
    // it is an error if the change is not in the 'being_reviewed' state.
    //
    if (cstate_data->state != cstate_state_being_reviewed)
        change_fatal(cp, 0, i18n("bad rp state"));
    assert(cstate_data->history->length >= 3);
    hp = cstate_data->history->list[cstate_data->history->length - 1];
    if (hp->what == cstate_history_what_review_begin)
    {
        if (nstring(hp->who) != up->name())
            change_fatal(cp, 0, i18n("not reviewer"));
    }
    else
    {
        pp = cp->pp;
        if (!project_reviewer_query(pp, up->name()))
            project_fatal(pp, 0, i18n("not a reviewer"));
        if (change_reviewer_already(cp, up->name()))
        {
            change_fatal(cp, 0, i18n("duplicate review"));
        }
        if
        (
            !project_developer_may_review_get(pp)
        &&
            nstring(cp->developer_name()) == up->name()
        )
        {
            change_fatal(cp, 0, i18n("developer may not review"));
        }
    }
}


static void
review_pass_main(void)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    long            j;
    edit_ty         edit;

    trace(("review_pass_main()\n{\n"));
    arglex();
    change_identifier cid;
    edit = edit_not_set;
    nstring comment;
    nstring reason;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(review_pass_usage);
            continue;

        case arglex_token_change:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_string:
            cid.command_line_parse(review_pass_usage);
            continue;

        case arglex_token_file:
            if (!comment.empty())
                duplicate_option(review_pass_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, review_pass_usage);
                // NOTREACHED

            case arglex_token_string:
                os_become_orig();
                comment = read_whole_file(arglex_value.alv_string);
                os_become_undo();
                break;

            case arglex_token_stdio:
                os_become_orig();
                comment = read_whole_file((char *)0);
                os_become_undo();
                break;
            }
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(review_pass_usage);
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_reason, review_pass_usage);

            case arglex_token_string:
            case arglex_token_number:
                reason = arglex_value.alv_string;
                break;
            }
            break;

        case arglex_token_edit:
            if (edit == edit_foreground)
                duplicate_option(review_pass_usage);
            if (edit != edit_not_set)
            {
                too_many_edits:
                mutually_exclusive_options
                (
                    arglex_token_edit,
                    arglex_token_edit_bg,
                    review_pass_usage
                );
            }
            edit = edit_foreground;
            break;

        case arglex_token_edit_bg:
            if (edit == edit_background)
                duplicate_option(review_pass_usage);
            if (edit != edit_not_set)
                goto too_many_edits;
            edit = edit_background;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(review_pass_usage);
            break;

        case arglex_token_signed_off_by:
        case arglex_token_signed_off_by_not:
            option_signed_off_by_argument(review_pass_usage);
            break;
        }
        arglex();
    }
    cid.command_line_check(review_pass_usage);

    if (!comment.empty() && !reason)
    {
        mutually_exclusive_options
        (
            arglex_token_file,
            arglex_token_reason,
            review_pass_usage
        );
    }
    if (edit != edit_not_set && (!comment.empty() || !reason.empty()))
    {
        mutually_exclusive_options
        (
            (
                edit == edit_foreground
            ?
                arglex_token_edit
            :
                arglex_token_edit_bg
            ),
            (comment.empty() ? arglex_token_reason : arglex_token_file),
            review_pass_usage
        );
    }
    if (!reason.empty())
        comment = reason;

    project *pp = cid.get_pp();
    user_ty::pointer up = cid.get_up();
    change::pointer cp = cid.get_cp();

    //
    // create the comment, if necessary
    // check permissions first
    //
    if (edit != edit_not_set)
    {
        check_permissions(cp, up);
        comment = nstring(os_edit_new(edit));
    }

    //
    // lock the change for writing
    //
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // it is an error if the change is not in the 'being_reviewed' state.
    //
    check_permissions(cp, up);

    //
    // change the state
    // remember who reviewed it
    // add to the change's history
    //
    cstate_data->state = cstate_state_awaiting_integration;
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_review_pass;
    if (!comment.empty())
        history_data->why = str_copy(comment.get_ref());
    if  (0 != change_run_review_policy_command(cp, up))
    {
        //
        // The review policy command failed.
        //
        // This means that more review is required.  Now we have to work
        // out which state to send the change to.
        //
        // (If the review policy command hasn't been set, the above
        // function returns zero, and you never get to here.)
        //
        switch (project_develop_end_action_get(pp))
        {
        case cstate_branch_develop_end_action_goto_awaiting_review:
            cstate_data->state = cstate_state_awaiting_review;
            history_data->what = cstate_history_what_review_pass_2ar;
            break;

        case cstate_branch_develop_end_action_goto_awaiting_integration:
            //
            // This unlikely event happens when there were
            // changes in awaiting_review and/or being_reviewed
            // when the develop_end_action was changed to
            // goto_awaiting_integration.
            //
            // Since we don't know that the develop_end_action was
            // before, we assume it was the default, and fall through...
            //
        case cstate_branch_develop_end_action_goto_being_reviewed:
            cstate_data->state = cstate_state_being_reviewed;
            history_data->what = cstate_history_what_review_pass_2br;
            break;

#ifndef DEBUG
        default:
            //
            // Something weird is going on.
            // Advance to being_integrated anyway.
            //
            assert(0);
            break;
#endif
        }
    }

    //
    // It is an error if any of the change files have been tampered
    // with, or any of the difference files.
    //
    for (j = 0;; ++j)
    {
        fstate_src_ty   *src_data;
        string_ty       *path;
        string_ty       *path_d;
        int             same;
        int             file_required;

        src_data = change_file_nth(cp, j, view_path_first);
        if (!src_data)
            break;

        file_required = 1;
        bool diff_file_required = change_diff_required(cp);
        switch (src_data->usage)
        {
        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;

        case file_usage_build:
            file_required = 0;
            diff_file_required = false;
            break;
        }
        switch (src_data->action)
        {
        case file_action_create:
        case file_action_modify:
            break;

        case file_action_insulate:
            assert(0);
            break;

        case file_action_transparent:
            if (cp->was_a_branch())
            {
                file_required = 0;
                diff_file_required = false;
            }
            break;

        case file_action_remove:
            file_required = 0;

            //
            // the removed half of a move is not differenced
            //
            if
            (
                src_data->move
            &&
                cp->file_find(nstring(src_data->move), view_path_first)
            )
                diff_file_required = false;
            break;
        }

        path = cp->file_path(src_data->file_name);
        if (file_required)
        {
            up->become_begin();
            same = change_fingerprint_same(src_data->file_fp, path, 0);
            up->become_end();
            if (!same)
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_string(scp, "File_Name", src_data->file_name);
                change_fatal(cp, scp, i18n("$filename altered"));
                // NOTREACHED
                sub_context_delete(scp);
            }
        }

        path_d = str_format("%s,D", path->str_text);
        if (diff_file_required)
        {
            up->become_begin();
            same = change_fingerprint_same(src_data->diff_file_fp, path_d, 0);
            up->become_end();
            if (!same)
            {
                sub_context_ty  *scp;

                scp = sub_context_new();
                sub_var_set_format
                (
                    scp,
                    "File_Name",
                    "%s,D",
                    src_data->file_name->str_text
                );
                change_fatal(cp, scp, i18n("$filename altered"));
                // NOTREACHED
                sub_context_delete(scp);
            }
        }
        str_free(path);
        str_free(path_d);
    }

    //
    // If the project is configured to use Signed-off-by lines in
    // change descriptions, append a Signed-off-line to this change's
    // description.
    //
    if (change_signed_off_by_get(cp))
        change_signed_off_by(cp, up);

    //
    // write out the data and release the locks
    //
    cp->cstate_write();
    commit();
    lock_release();

    //
    // run the notify command
    //
    cp->run_review_pass_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("review pass complete"));
    trace(("}\n"));
}


void
review_pass(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, review_pass_help, 0 },
        { arglex_token_list, review_pass_list, 0 },
    };

    trace(("review_pass()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_pass_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
