//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/mem.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/pattr.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aerb.h>


static void
review_begin_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -Review_Begin <change_number> [ <option>... ]\n",
        progname
    );
    fprintf
    (
        stderr,
        "       %s -Review_Begin -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Review_Begin -Help\n", progname);
    quit(1);
}


static void
review_begin_help(void)
{
    help("aerb", review_begin_usage);
}


static void
review_begin_list(void)
{
    trace(("review_begin_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(review_begin_usage);
    int mask = 1 << cstate_state_awaiting_review;

    //
    // It's messy.  Depending on the "develop end action" project
    // attribute, we have to cope with both "awaiting review" and
    // "being reviewed" states.
    //
    // This is because when the action is "goto_being_reviewed"
    // then the aerb command still runs the "review_begin_notify_
    // command", which is usually used to send email (or news)
    // to other reviewers and thus avoid duplicating effort.
    //
    if
    (
        project_develop_end_action_get(cid.get_pp())
    ==
        pattr_develop_end_action_goto_being_reviewed
    )
    {
        mask |= 1 << cstate_state_being_reviewed;
    }

    list_changes_in_state_mask(cid, mask);
    trace(("}\n"));
}


static void
review_begin_main(void)
{
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change::pointer cp;
    user_ty::pointer up;
    long            j;

    trace(("review_begin_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(review_begin_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                review_begin_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, review_begin_usage);
            continue;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(review_begin_usage);
            break;

	case arglex_token_reason:
	    if (reason)
		duplicate_option(review_begin_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string(arglex_token_reason, review_begin_usage);
		// NOTREACHED

	    case arglex_token_string:
	    case arglex_token_number:
		reason = str_from_c(arglex_value.alv_string);
		break;
	    }
	    break;
        }
        arglex();
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
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

    //
    // lock the change for writing
    //
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // it is not an error if
    //      - the change is in the 'awaiting_review' state, OR
    //      - the "develop end action" is "goto being reviewed"
    //        and the change is in the 'being_reviewed' state.
    //
    if
    (
        cstate_data->state != cstate_state_awaiting_review
    &&
        (
                project_develop_end_action_get(pp)
            !=
                pattr_develop_end_action_goto_being_reviewed
        ||
            cstate_data->state != cstate_state_being_reviewed
        )
    )
        change_fatal(cp, 0, i18n("bad rb state"));
    if (!project_reviewer_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not a reviewer"));
    if
    (
        !project_developer_may_review_get(pp)
    &&
        nstring(change_developer_name(cp)) == up->name()
    )
        change_fatal(cp, 0, i18n("developer may not review"));

    //
    // change the state
    // remember who reviewed it
    // add to the change's history
    //
    // Don't do anything if the change is already in the being_reviewed state.
    //
    if (cstate_data->state != cstate_state_being_reviewed)
    {
        cstate_data->state = cstate_state_being_reviewed;
        history_data = change_history_new(cp, up);
        history_data->what = cstate_history_what_review_begin;
	history_data->why = reason;
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
        case file_usage_build:
            file_required = 0;
            diff_file_required = false;
            break;

        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;
        }
        switch (src_data->action)
        {
        case file_action_remove:
            file_required = 0;

            //
            // the removed half of a move is not differenced
            //
            if
            (
                src_data->move
            &&
                change_file_find(cp, src_data->move, view_path_first)
            )
                diff_file_required = false;
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
        case file_action_transparent:
            break;
        }

        path = change_file_path(cp, src_data->file_name);
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
	trace_bool(diff_file_required);
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
    // write out the data and release the locks
    //
    change_cstate_write(cp);
    commit();
    lock_release();

    //
    // run the notify command
    //
    cp->run_review_begin_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("review begin complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
review_begin(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, review_begin_help, 0 },
        { arglex_token_list, review_begin_list, 0 },
    };

    trace(("review_begin()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), review_begin_main);
    trace(("}\n"));
}
