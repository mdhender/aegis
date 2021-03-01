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
#include <common/ac/time.h>

#include <common/progname.h>
#include <common/quit.h>
#include <common/sizeof.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/ael/change/changes.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/cattr.fmtgen.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change/verbose.h>
#include <libaegis/col.h>
#include <libaegis/commit.h>
#include <libaegis/common.fmtgen.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/io.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aegis/aeca.h>
#include <aegis/aenc.h>


static void
new_change_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
        stderr,
        "usage: %s -New_Change -File <attr-file> [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -New_Change -Edit [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Change -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -New_Change -Help\n", progname);
    quit(1);
}


static void
new_change_help(void)
{
    help("aenc", new_change_usage);
}


static void
new_change_list(void)
{
    trace(("new_chane_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(new_change_usage);
    list_changes(cid, 0);
    trace(("}\n"));
}


void
new_change_check_permission(project *pp, user_ty::pointer up)
{
    //
    // it is an error if
    // the user is not an administrator for the project.
    //
    if
    (
        !project_administrator_query(pp, up->name())
    &&
        (
            !project_developers_may_create_changes_get(pp)
        ||
            !project_developer_query(pp, up->name())
        )
    )
    {
        project_fatal(pp, 0, i18n("not an administrator"));
    }
}


static void
new_change_main(void)
{
    sub_context_ty  *scp;
    cstate_ty       *cstate_data;
    cstate_history_ty *history_data;
    cattr_ty        *cattr_data;
    string_ty       *project_name;
    project      *pp;
    long            change_number;
    change::pointer cp;
    user_ty::pointer up;
    edit_ty         edit;
    size_t          j;
    pconf_ty        *pconf_data;
    const char      *output_filename;
    string_ty       *inp;

    trace(("new_change_main()\n{\n"));
    arglex();
    cattr_data = 0;
    project_name = 0;
    edit = edit_not_set;
    change_number = 0;
    output_filename = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(new_change_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                new_change_usage
            );
            continue;

        case arglex_token_string:
            scp = sub_context_new();
            sub_var_set_charstar
            (
                scp,
                "Name",
                arglex_token_name(arglex_token_file)
            );
            error_intl(scp, i18n("warning: use $name option"));
            sub_context_delete(scp);
            if (cattr_data)
                fatal_too_many_files();
            goto read_input_file;

        case arglex_token_file:
            if (cattr_data)
                duplicate_option(new_change_usage);
            inp = 0;
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, new_change_usage);
                // NOTREACHED

            case arglex_token_string:
                read_input_file:
                inp = str_from_c(arglex_value.alv_string);
                break;

            case arglex_token_stdio:
                inp = str_from_c("");
                break;
            }
            os_become_orig();
            cattr_data = cattr_read_file(inp);
            os_become_undo();
            assert(cattr_data);
            change_attributes_fixup(cattr_data);
            change_attributes_verify(inp, cattr_data);
            break;

        case arglex_token_project:
            arglex();
            arglex_parse_project(&project_name, new_change_usage);
            continue;

        case arglex_token_edit:
            if (edit == edit_foreground)
                duplicate_option(new_change_usage);
            if (edit != edit_not_set)
            {
                too_many_edits:
                mutually_exclusive_options
                (
                    arglex_token_edit,
                    arglex_token_edit_bg,
                    new_change_usage
                );
            }
            edit = edit_foreground;
            break;

        case arglex_token_edit_bg:
            if (edit == edit_background)
                duplicate_option(new_change_usage);
            if (edit != edit_not_set)
                goto too_many_edits;
            edit = edit_background;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(new_change_usage);
            break;

        case arglex_token_output:
            if (output_filename)
                duplicate_option(new_change_usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_output, new_change_usage);
                // NOTREACHED

            case arglex_token_string:
                output_filename = arglex_value.alv_string;
                break;

            case arglex_token_stdio:
                output_filename = "";
                break;
            }
            break;

        case arglex_token_reason:
            if (reason)
                duplicate_option(new_change_usage);
            switch (arglex())
            {
            default:
                option_needs_string(arglex_token_reason, new_change_usage);
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
    if (change_number && output_filename)
    {
        mutually_exclusive_options
        (
            arglex_token_change,
            arglex_token_output,
            new_change_usage
        );
    }
    if (edit != edit_not_set && cattr_data)
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
            arglex_token_file,
            new_change_usage
        );
    }
    if (edit == edit_not_set && !cattr_data)
    {
        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Name1",
            arglex_token_name(arglex_token_file)
        );
        sub_var_set_charstar
        (
            scp,
            "Name2",
            arglex_token_name(arglex_token_edit)
        );
        error_intl(scp, i18n("warning: no $name1, assuming $name2"));
        sub_context_delete(scp);
        edit = edit_foreground;
    }

    //
    // locate project data
    //
    if (!project_name)
        fatal_intl(0, i18n("no project name"));
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // make sure this branch of the project is still active
    //
    if (!pp->change_get()->is_a_branch())
        project_fatal(pp, 0, i18n("branch completed"));

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // see if must invoke editor
    //
    if (edit != edit_not_set)
    {
        //
        // make sure they are allowed to,
        // to avoid a wasted edit
        //
        new_change_check_permission(pp, up);

        //
        // build template cattr
        //
        if (!cattr_data)
        {
            string_ty       *none;

            none = str_from_c("none");
            cattr_data = (cattr_ty *)cattr_type.alloc();
            cattr_data->brief_description = str_copy(none);
            cattr_data->description = str_copy(none);
            cattr_data->cause = change_cause_internal_bug;
            str_free(none);
        }

        //
        // default a few things
        //      (create a fake change to extract the pconf)
        //
        cp = change_alloc(pp, TRUNK_CHANGE_NUMBER - 1);
        change_bind_new(cp);
        cstate_data = cp->cstate_get();
        cstate_data->state = cstate_state_awaiting_development;
        pconf_data = change_pconf_get(cp, 0);
        change_attributes_default(cattr_data, pp, pconf_data);
        change_free(cp);

        //
        // edit the attributes
        //
        scp = sub_context_new();
        sub_var_set_string(scp, "Name", project_name_get(pp));
        io_comment_append(scp, i18n("Project $name"));
        io_comment_append(scp, i18n("nc dflt hint"));
        sub_context_delete(scp);
        change_attributes_edit(&cattr_data, edit);
    }

    //
    // Lock the project state file.
    // Block if necessary.
    //
    pp->pstate_lock_prepare();
    lock_take();

    //
    // make sure they are allowed to
    // (even if edited, may have changed while editing)
    //
    new_change_check_permission(pp, up);

    //
    // Add another row to the change table.
    //
    if (!change_number)
        change_number = project_next_change_number(pp, 1);
    else
    {
        if (project_change_number_in_use(pp, change_number))
        {
            scp = sub_context_new();
            sub_var_set_long(scp, "Number", magic_zero_decode(change_number));
            project_fatal(pp, scp, i18n("change $number used"));
            // NOTREACHED
            sub_context_delete(scp);
        }
    }
    cp = change_alloc(pp, change_number);
    change_bind_new(cp);
    cstate_data = cp->cstate_get();
    cstate_data->state = cstate_state_awaiting_development;
    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->build_command)
    {
        //
        // There is no build command.  This means that the project
        // does not yet have a "config" file.  From this,
        // infer that this is the first change of the project.
        //
        // It has to be the first change because: aeb will fail
        // when it can't find a build_command in the non-existent
        // config file, so you can't aede until there is a
        // valid config file, so you can't have a baseline with
        // *anything* in it unless it also has a valid config
        // file.  Ergo, there have been no integrations yet;
        // we must be the first change.
        //
        // There could be a couple of changes created before one
        // of them is sucessfully integrated, but it doesn't
        // happen all that often, so I'm not going to worry
        // about it.
        //
        cattr_data->cause = change_cause_internal_enhancement;
        cattr_data->test_baseline_exempt = true;
        cattr_data->mask |= cattr_test_baseline_exempt_mask;
        cattr_data->regression_test_exempt = true;
        cattr_data->mask |= cattr_regression_test_exempt_mask;
    }
    change_attributes_default(cattr_data, pp, pconf_data);

    //
    // when developers create changes,
    // they may not give themselves a testing exemption,
    // or a architecture exemption,
    // only administrators may do that.
    //
    // Don't worry about this stuff for the very first change of
    // a project.
    //
    if
    (
        pconf_data->build_command
    &&
        !project_administrator_query(pp, up->name())
    )
    {
        cattr_ty        *dflt;

        //
        // If they are asking for default behaviour, don't complain.
        // (e.g. admin may have given general testing exemption)
        //
        dflt = (cattr_ty *)cattr_type.alloc();
        dflt->cause = cattr_data->cause;
        change_attributes_default(dflt, pp, pconf_data);

        if
        (
            (
                (cattr_data->mask & cattr_test_exempt_mask)
            &&
                cattr_data->test_exempt
            &&
                (cattr_data->test_exempt != dflt->test_exempt)
            )
        ||
            (
                (cattr_data->mask & cattr_test_baseline_exempt_mask)
            &&
                cattr_data->test_baseline_exempt
            &&
                (cattr_data->test_baseline_exempt != dflt->test_baseline_exempt)
            )
        ||
            (
                (cattr_data->mask & cattr_regression_test_exempt_mask)
            &&
                cattr_data->regression_test_exempt
            &&
                (
                    cattr_data->regression_test_exempt
                !=
                    dflt->regression_test_exempt
                )
            )
        )
            fatal_intl(0, i18n("bad ca, no test exempt"));
        assert(cattr_data->architecture);
        assert(cattr_data->architecture->length);
        assert(dflt->architecture);
        assert(dflt->architecture->length);

        string_list_ty carch;
        for (j = 0; j < cattr_data->architecture->length; ++j)
            carch.push_back(cattr_data->architecture->list[j]);
        string_list_ty darch;
        for (j = 0; j < dflt->architecture->length; ++j)
            darch.push_back(dflt->architecture->list[j]);
        if (carch != darch)
            fatal_intl(0, i18n("bad ca, no arch exempt"));
        cattr_type.free(dflt);
    }

    //
    // make sure the architecture list only covers
    // variations in the project's architecture list
    //
    assert(cattr_data->architecture);
    string_list_ty carch;
    for (j = 0; j < cattr_data->architecture->length; ++j)
        carch.push_back(cattr_data->architecture->list[j]);
    assert(pconf_data->architecture);
    string_list_ty parch;
    for (j = 0; j < pconf_data->architecture->length; ++j)
        parch.push_back(pconf_data->architecture->list[j]->name);
    if (!carch.subset(parch))
        fatal_intl(0, i18n("bad ca, unknown architecture"));

    //
    // set change state from the attributes
    // Create the change history.
    //
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_new_change;
    if (reason)
        history_data->why = reason;
    if (cattr_data->description)
        cstate_data->description = str_copy(cattr_data->description);
    assert(cattr_data->brief_description);
    cstate_data->brief_description = str_copy(cattr_data->brief_description);
    assert(cattr_data->mask & cattr_cause_mask);
    cstate_data->cause = cattr_data->cause;
    assert(cattr_data->mask & cattr_test_exempt_mask);
    cstate_data->test_exempt = cattr_data->test_exempt;
    assert(cattr_data->mask & cattr_test_baseline_exempt_mask);
    cstate_data->test_baseline_exempt = cattr_data->test_baseline_exempt;
    assert(cattr_data->mask & cattr_regression_test_exempt_mask);
    cstate_data->regression_test_exempt = cattr_data->regression_test_exempt;
    cstate_data->given_regression_test_exemption =
        cattr_data->regression_test_exempt;
    cstate_data->given_test_exemption = cattr_data->test_exempt;
    change_architecture_clear(cp);
    for (j = 0; j < cattr_data->architecture->length; ++j)
        change_architecture_add(cp, cattr_data->architecture->list[j]);
    if (cattr_data->attribute)
        cstate_data->attribute = attributes_list_copy(cattr_data->attribute);
    cattr_type.free(cattr_data);

    //
    // Write out the change file.
    // There is no need to lock this file
    // as it does not exist yet;
    // the project state file, with the number in it, is locked.
    //
    cp->cstate_write();

    //
    // Add the change to the list of existing changes.
    // Increment the next_change_number.
    // and write pstate back out.
    //
    project_change_append(pp, change_number, 0);

    //
    // If there is an output option,
    // write the change number to the file.
    //
    if (output_filename)
    {
        string_ty       *content;

        content = str_format("%ld", magic_zero_decode(change_number));
        if (*output_filename)
        {
            string_ty       *fn;

            user_ty::become scoped(up);
            fn = str_from_c(output_filename);
            file_from_string(fn, content, 0644);
            str_free(fn);
        }
        else
            cat_string_to_stdout(content);
        str_free(content);
    }

    //
    // Unlock the pstate file.
    //
    pp->pstate_write();
    commit();
    lock_release();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose_new_change_complete(cp);
    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}


void
new_change(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, new_change_help, 0 },
        { arglex_token_list, new_change_list, 0 },
    };

    trace(("new_change()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), new_change_main);
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
