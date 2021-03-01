//
//      aegis - project change supervisor
//      Copyright (C) 1991-2008 Peter Miller
//      Copyright (C) 2006-2008 Walter Franzini
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>
#include <common/ac/unistd.h>

#include <common/env.h>
#include <common/error.h>
#include <common/now.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change/pfs.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/glue.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/metrics.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/quit/action/histtransabt.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aeip.h>


#define MTIME_BLURB 0

struct time_map_ty
{
    time_t          old;
    time_t          becomes;
};

struct time_map_list_ty
{
    time_map_ty     *list;
    size_t          len;
    size_t          max_len;
    time_t          time_aeib;
    time_t          time_aeip;
};


static void
integrate_pass_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Integrate_PASS [ <option>... ]\n", progname);
    fprintf
    (
        stderr,
        "       %s -Integrate_PASS -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Integrate_PASS -Help\n", progname);
    quit(1);
}


static void
integrate_pass_help(void)
{
    help("aeipass", integrate_pass_usage);
}


static void
integrate_pass_list(void)
{
    trace(("integrate_pass_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(integrate_pass_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_being_integrated);
    trace(("}\n"));
}


static void
time_map_get(void *p, dir_walk_message_ty message, string_ty *,
    const struct stat *st)
{
    time_map_list_ty *tlp;
    time_map_ty     *tp;
    time_t          t;

    //
    // If it's not a file, ignore it.
    // If its mtime is before aeib, ignore it.
    //
    if (message != dir_walk_file)
        return;
    tlp = (time_map_list_ty *)p;
    t = st->st_mtime;
    if (t < tlp->time_aeib)
        return;

    //
    // Find the time in the list using a binary chop.
    // If it is already there, do nothing.
    //
    long min_idx = 0;
    long max_idx = (long)tlp->len - 1;
    while (min_idx <= max_idx)
    {
        long mid = (min_idx + max_idx) / 2;
        time_t mid_t = tlp->list[mid].old;
        if (mid_t == t)
            return;
        if (mid_t < t)
            min_idx = mid + 1;
        else
            max_idx = mid - 1;
    }

    //
    // This is a new time, insert it into the list sorted by time.
    //
    assert(min_idx >= 0);
    if (tlp->len >= tlp->max_len)
    {
        tlp->max_len = tlp->max_len * 2 + 16;
	time_map_ty *new_list = new time_map_ty [tlp->max_len];
	for (size_t k = 0; k < size_t(min_idx); ++k)
	    new_list[k] = tlp->list[k];
	for (size_t j = min_idx; j < tlp->len; ++j)
	    new_list[j + 1] = tlp->list[j];
	delete [] tlp->list;
	tlp->list = new_list;
    }
    else
    {
	for (size_t j = tlp->len; j > size_t(min_idx); --j)
	    tlp->list[j] = tlp->list[j - 1];
    }
    tlp->len++;
    tp = &tlp->list[min_idx];
    tp->old = st->st_mtime;
    tp->becomes = st->st_mtime;
}


static void
time_map_set(void *p, dir_walk_message_ty message, string_ty *path,
    const struct stat *st)
{
    time_map_list_ty *tlp;
    time_t          t;

    //
    // If it's not a file, ignore it.
    // If its mtime is before aeib, ignore it.
    //
    if (message != dir_walk_file)
        return;
    tlp = (time_map_list_ty *)p;
    t = st->st_mtime;
    if (t < tlp->time_aeib)
        return;

    //
    // Find the time in the list using a binary chop.
    //
    long min_idx = 0;
    long max_idx = (long)tlp->len - 1;
    while (min_idx <= max_idx)
    {
        long mid = (min_idx + max_idx) / 2;
        time_t mid_t = tlp->list[mid].old;
        if (mid_t == t)
        {
            min_idx = mid;
            max_idx = mid;
            break;
        }
        if (mid_t < t)
            min_idx = mid + 1;
        else
            max_idx = mid - 1;
    }

    //
    // If the file time has been altered since aeipass began (and
    // thus is not in the list), which hopefully is *very* rare, as
    // it requires direct human interference, use a close time.
    //
    if (min_idx >= (long)tlp->len)
    {
        min_idx = (long)tlp->len - 1;
    }

    //
    // set the file time
    //
    os_mtime_set_errok(path, tlp->list[min_idx].becomes);
}


static void
commit_unlink_symlinks(string_ty *path)
{
#if defined(S_IFLNK) || defined(S_ISLNK)
    string_list_ty  wl;
    struct stat     st;
    size_t          j;
    string_ty       *s;

    trace(("commit_unlink_symlinks(path = %08lX)\n{\n", (long)path));
    assert(path);
    trace_string(path->str_text);

    //
    // Read the directory contents.
    //
    if (read_whole_dir__wl(path->str_text, &wl))
    {
        sub_context_ty  *scp;
        int             errno_old;

        errno_old = errno;
        scp = sub_context_new();
        sub_errno_setx(scp, errno_old);
        sub_var_set_string(scp, "File_Name", path);
        fatal_intl(scp, i18n("read $filename: $errno"));
        // NOTREACHED
    }

    //
    // For each file in the directory, unlink it IF it's a symlink.
    //
    for (j = 0; j < wl.nstrings; ++j)
    {
        s = os_path_cat(path, wl.string[j]);
        if (glue_lstat(s->str_text, &st))
        {
            sub_context_ty      *scp;
            int             errno_old;

            errno_old = errno;
            scp = sub_context_new();
            sub_errno_setx(scp, errno_old);
            sub_var_set_string(scp, "File_Name", path);
            fatal_intl(scp, i18n("lstat $filename: $errno"));
            // NOTREACHED
        }
        if (S_ISLNK(st.st_mode))
            commit_unlink_errok(s);
        str_free(s);
    }
    trace(("}\n"));
#endif
}


static bool
reasonable_number_of_test_correlations(change::pointer cp)
{
    size_t nsources = 0;
    size_t ntests = 0;
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;
	switch (src->action)
	{
	case file_action_create:
	case file_action_modify:
	case file_action_transparent:
	case file_action_insulate:
	    switch (src->usage)
	    {
	    case file_usage_build:
	    case file_usage_config:
		break;

	    case file_usage_source:
		++nsources;
		break;

	    case file_usage_test:
	    case file_usage_manual_test:
		++ntests;
		break;
	    }
	    break;

	case file_action_remove:
	    break;
	}
    }
    return (nsources * ntests <= 1000);
}


static void
integrate_pass_main(void)
{
    time_t          mtime;
    time_t          youngest;
    string_ty       *id;
    string_ty       *cwd;
    cstate_ty       *cstate_data;
    string_ty       *old_baseline;
    string_ty       *new_baseline;
    string_ty       *dev_dir;
    string_ty       *dev_name;
    cstate_history_ty *history_data;
    size_t          j;
    size_t          k;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change::pointer cp;
    log_style_ty    log_style;
    user_ty::pointer up;
    user_ty::pointer devup;
    user_ty::pointer pup;
    int             nerr;
    int             diff_whine;
    change::pointer pcp;
    cstate_ty       *p_cstate_data;
    time_map_list_ty tml;

    trace(("integrate_pass_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    nerr = 0;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(integrate_pass_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                integrate_pass_usage
            );
            continue;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, integrate_pass_usage);
            continue;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(integrate_pass_usage);
            log_style = log_style_none;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(integrate_pass_usage);
            break;

	case arglex_token_reason:
	    if (reason)
		duplicate_option(integrate_pass_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string(arglex_token_reason, integrate_pass_usage);
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
    pup = project_user(pp);

    //
    // locate change data
    //
    if (!change_number)
        change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Take an advisory write lock on the appropriate row of the change
    // table.  Take an advisory write lock on the appropriate row of the
    // user table.  Block until can get both simultaneously.
    //
    // Also take a write lock on the baseline, to ensure that it does
    // not change for the duration of builds (aeb) or file copies (aecp).
    // And a lock on the history files, so that no aeip can trash
    // another's history files.
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    up->ustate_lock_prepare();
    project_baseline_write_lock_prepare(pp);
    project_history_lock_prepare(pp);
    lock_take();
    cstate_data = cp->cstate_get();

    //
    // It is an error if the change is not in the being_integrated state.
    // It is an error if the change is not assigned to the current user.
    //
    if (cstate_data->state != cstate_state_being_integrated)
        change_fatal(cp, 0, i18n("bad ip state"));
    if (nstring(change_integrator_name(cp)) != up->name())
        change_fatal(cp, 0, i18n("not integrator"));
    nerr = 0;
    youngest = 0;
    os_throttle();

    //
    // Assign the change a UUID if it doesn't have one already.
    //
    change_uuid_set(cp);

    //
    // add to history for state change
    //
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_pass;
    history_data->why = reason;
    tml.time_aeip = history_data->when;
#if MTIME_BLURB
    error_raw("lock taken %.24s", ctime(&tml.time_aeip));
#endif

    //
    // add to project history
    //
    project_history_new(pp, cstate_data->delta_number, change_number);

    //
    // Find the most recent aeib in the history, and remember when
    // that was, so we can leave file mtimes alone when they fall
    // before that point.
    //
    assert(cstate_data->history);
    assert(cstate_data->history->length);
    tml.time_aeib = 0;
    for (j = 0; j < cstate_data->history->length; ++j)
    {
        cstate_history_ty *hhp;

        hhp = cstate_data->history->list[j];
        if (hhp->what == cstate_history_what_integrate_begin)
            tml.time_aeib = hhp->when;
    }
    assert(tml.time_aeib);

    id = change_integration_directory_get(cp, 1);

    //
    // Set this environment variable for the integrate_pass_notify_command
    // which is run after the name is gone from the database.
    //
    env_set("AEGIS_INTEGRATION_DIRECTORY", id->str_text);

    //
    // Only update the test correlations if they are reasonable.
    //
    // The problem is that for a change with thousands of files and
    // hundreds of tests, we add multi-megabytes of fundamentally
    // useless test correlations.  For example, updating copyright
    // notices after a corporate merger.
    //
    bool update_test_correlations = reasonable_number_of_test_correlations(cp);

    //
    // Walk the change files, making sure
    //      1. the change has been diffed (except for the trunk's baseline)
    //      2. parent files are copied into the change
    //      3. test times are transferred
    //      4. The fingerprint is still correct.
    //
    pcp = pp->change_get();
    diff_whine = 0;
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    nstring_list removed_file;
    for (j = 0;; ++j)
    {
        fstate_src_ty   *c_src_data;
        fstate_src_ty   *p_src_data;
        int             transfer_architecture_times;
        int             transfer_file_times;
        int             transfer_diff_file_times;

        c_src_data = change_file_nth(cp, j, view_path_first);
        if (!c_src_data)
            break;
        trace(("file_name = \"%s\"\n", c_src_data->file_name->str_text));

	//
	// Check for unchanged files.
	//
	switch (pconf_data->unchanged_file_integrate_pass_policy)
	{
	case pconf_unchanged_file_integrate_pass_policy_ignore:
	    // Don't look for unchanged files.
	    break;

	case pconf_unchanged_file_integrate_pass_policy_warning:
	    if (change_file_unchanged(cp, c_src_data, up))
	    {
		//
                // Warn about unchanged files in the change set, but
                // leave them in the change set.
		//
		sub_context_ty sc;
		sc.var_set_string("File_Name", c_src_data->file_name);
		change_warning(cp, &sc, i18n("$filename unchanged"));
	    }
	    break;

	case pconf_unchanged_file_integrate_pass_policy_remove:
	    if (change_file_unchanged(cp, c_src_data, up))
	    {
		//
		// Silently remove unchanged files from the change set.
		// UNLESS that would leave the change with no files at all.
		//
		if (j != 0 || change_file_nth(cp, 1, view_path_first))
		{
		    change_file_remove(cp, c_src_data->file_name);
                    removed_file.push_back(nstring(c_src_data->file_name));
		    --j;
		    continue;
		}
	    }
	    break;
	}

        //
        // check the the file has been differenced
        //
	if (!diff_whine)
	{
	    if (change_diff_required(cp))
	    {
		bool diff_required = true;
		switch (c_src_data->action)
		{
		case file_action_remove:
		    if
		    (
			c_src_data->move
		    &&
			change_file_find(cp, c_src_data->move, view_path_first)
		    )
		    {
			diff_required = false;
			break;
		    }
		    // fall through...

		case file_action_create:
		case file_action_modify:
#ifndef DEBUG
		default:
#endif
		    if (pp->is_a_trunk())
		    {
			diff_required = false;
			break;
		    }
		    if (c_src_data->usage == file_usage_build)
		    {
			diff_required = false;
			break;
		    }

		    if
		    (
			c_src_data->idiff_file_fp
		    &&
			c_src_data->idiff_file_fp->youngest
		    )
			diff_required = false;
		    break;

		case file_action_insulate:
		case file_action_transparent:
		    diff_required = false;
		    break;
		}
		if (diff_required)
		{
		    change_error(cp, 0, i18n("diff required"));
		    ++nerr;
		    ++diff_whine;
		}
	    }
	}

        //
        // For each change file that is acting on a project file
        // from a deeper level than the immediate parent
        // project, the file needs to be added to the immediate
        // parent project.
        //
        // This should already have been done by aede.
        //
        trace(("shallow checking \"%s\"\n", c_src_data->file_name->str_text));
        if (!project_file_shallow_check(pp, c_src_data->file_name))
            this_is_a_bug();

        p_src_data =
            change_file_find(pcp, c_src_data->file_name, view_path_first);
        if (!p_src_data)
            this_is_a_bug();

        //
        // The file may be having its usage field changed, even though
        // it's marked "modify" (only the meta-data is changing).
        //
        switch (c_src_data->action)
        {
        case file_action_modify:
        case file_action_create:
            p_src_data->usage = c_src_data->usage;
            break;

        case file_action_remove:
        case file_action_insulate:
        case file_action_transparent:
            break;
        }

        //
        // The project file's UUID tracks the change file's UUID.  If the
        // change file has none, after the integration the project file
        // will have none as well.
        //
        // The only case where we add a UUID is if the file is being
        // created.  In all other cases we must leave things alone, fo
        // backwards compatibility reasons: the history would become
        // inaccessable.
        //
        // Make sure this stays in synch with the code in
        // libaegis/change/file/cpy_bas_attr.c
        //
        if (p_src_data->uuid)
        {
            str_free(p_src_data->uuid);
            p_src_data->uuid = 0;
        }
        if
        (
            !c_src_data->uuid
        &&
            c_src_data->action == file_action_create
        &&
            !change_was_a_branch(cp)
        )
            c_src_data->uuid = universal_unique_identifier();
        if (c_src_data->uuid)
            p_src_data->uuid = str_copy(c_src_data->uuid);

        //
        // Transfer the file attributes, if any.
        //
        if (p_src_data->attribute)
        {
            attributes_list_type.free(p_src_data->attribute);
            p_src_data->attribute = 0;
        }
        if (c_src_data->attribute && c_src_data->attribute->length)
        {
            p_src_data->attribute =
                attributes_list_copy(c_src_data->attribute);
        }

        //
        // remove the file metrics, if any
        //
        if (c_src_data->metrics)
        {
            metric_list_type.free(c_src_data->metrics);
            c_src_data->metrics = 0;
        }
        if (p_src_data->metrics)
        {
            metric_list_type.free(p_src_data->metrics);
            p_src_data->metrics = 0;
        }

        //
        // Grab the file metrics,
        // if they were produced by the build.
        //
        // Only do this for primary source files, and only for
        // creates and modifies.
        //
        switch (c_src_data->usage)
        {
        case file_usage_build:
            break;

        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
#ifndef DEBUG
        default:
#endif
            switch (c_src_data->action)
            {
                metric_list_ty  *mlp;

            case file_action_create:
            case file_action_modify:
                mlp = change_file_metrics_get(cp, c_src_data->file_name);
                if (mlp)
                {
                    c_src_data->metrics = mlp;
                    p_src_data->metrics = metric_list_copy(mlp);
                }
                break;

            case file_action_insulate:
                assert(0);
                break;

            case file_action_remove:
            case file_action_transparent:
                break;
            }
            break;
        }

        //
        // don't do any of the transfers
        // if the file is built
        //
        transfer_architecture_times = 1;
        transfer_file_times = 1;
        transfer_diff_file_times = 1;
        switch (c_src_data->usage)
        {
        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
#ifndef DEBUG
        default:
#endif
            switch (c_src_data->action)
            {
            case file_action_create:
            case file_action_modify:
                break;

            case file_action_insulate:
                assert(0);
                // fall through...

            case file_action_transparent:
                transfer_architecture_times = 0;
                transfer_file_times = 0;
                transfer_diff_file_times = 0;
                break;

            case file_action_remove:
                transfer_architecture_times = 0;
                transfer_file_times = 0;
                if
                (
                    c_src_data->move
                &&
                    change_file_find(cp, c_src_data->move, view_path_first)
                )
                    transfer_diff_file_times = 0;
                break;
            }
            break;

        case file_usage_build:
            transfer_architecture_times = 0;
            transfer_file_times = 0;
            transfer_diff_file_times = 0;
            break;
        }

        //
        // Transfer the test times from the change
        // file to the project file.
        //
        if (p_src_data->architecture_times)
        {
            fstate_src_architecture_times_list_type.free
            (
                p_src_data->
                architecture_times
            );
            p_src_data->architecture_times = 0;
        }
        if (transfer_architecture_times && c_src_data->architecture_times)
        {
            p_src_data->architecture_times =
                (fstate_src_architecture_times_list_ty *)
                fstate_src_architecture_times_list_type.alloc();
            for (k = 0; k < c_src_data->architecture_times->length; ++k)
            {
                fstate_src_architecture_times_ty *catp;
                fstate_src_architecture_times_ty *patp;
                fstate_src_architecture_times_ty **addr;
                meta_type *type_p = 0;

                catp = c_src_data->architecture_times->list[k];
                addr =
                    (fstate_src_architecture_times_ty **)
                    fstate_src_architecture_times_list_type.list_parse
                    (
                        p_src_data->architecture_times,
                        &type_p
                    );
                assert(type_p == &fstate_src_architecture_times_type);
                patp =
                    (fstate_src_architecture_times_ty *)
                    fstate_src_architecture_times_type.alloc();
                *addr = patp;
                patp->variant = str_copy(catp->variant);
                patp->test_time = catp->test_time;
                patp->test_baseline_time = catp->test_baseline_time;
            }
        }

        //
        // Transfer the diff time from the change
        // file to the project file.
        //
        if (p_src_data->file_fp)
        {
            fingerprint_type.free(p_src_data->file_fp);
            p_src_data->file_fp = 0;
        }
        if (transfer_file_times)
        {
            //
            // It is possible that there is NO file fingerprint if
            // an old-format project had a change being integrated
            // when aegis was upgraded from 2.3 to 3.0.  Give an
            // error recommending that they start the integration
            // again.
            //
            if (!c_src_data->file_fp)
                change_fatal(cp, 0, i18n("restart integration after upgrade"));

            //
            // Transfer the file fingerprint.
            //
            // Note: the fp->youngest will be wrong, because
            // the file was copied into the integration
            // directory.  This will be fixed next time some
            // operation is done on this file, because the
            // crypto will be correct.
            //
            assert(c_src_data->file_fp);
            if (!p_src_data->file_fp)
            {
                p_src_data->file_fp =
                    (fingerprint_ty *)fingerprint_type.alloc();
            }
            assert(c_src_data->file_fp->youngest);
            p_src_data->file_fp->youngest = c_src_data->file_fp->youngest;
            assert(c_src_data->file_fp->oldest);
            p_src_data->file_fp->oldest = c_src_data->file_fp->oldest;
            if (p_src_data->file_fp->crypto)
                str_free(p_src_data->file_fp->crypto);
            assert(c_src_data->file_fp->crypto);
            p_src_data->file_fp->crypto = str_copy(c_src_data->file_fp->crypto);
        }

        //
        // Transfer the difference fingerprint.
        // Note the diff/idiff dichotomy.
        //
        if (p_src_data->diff_file_fp)
        {
            fingerprint_type.free(p_src_data->diff_file_fp);
            p_src_data->diff_file_fp = 0;
        }
        if (transfer_diff_file_times && c_src_data->idiff_file_fp)
        {
            if (!p_src_data->diff_file_fp)
            {
                p_src_data->diff_file_fp =
                    (fingerprint_ty *)fingerprint_type.alloc();
            }
            assert(c_src_data->idiff_file_fp->youngest);
            p_src_data->diff_file_fp->youngest =
                c_src_data->idiff_file_fp->youngest;
            assert(c_src_data->idiff_file_fp->oldest);
            p_src_data->diff_file_fp->oldest =
                c_src_data->idiff_file_fp->oldest;
            if (p_src_data->diff_file_fp->crypto)
                str_free(p_src_data->diff_file_fp->crypto);
            assert(c_src_data->idiff_file_fp->crypto);
            p_src_data->diff_file_fp->crypto =
                str_copy(c_src_data->idiff_file_fp->crypto);
        }

        //
        // Make sure the file fingerprint is still correct.
        // If it has changed, then the one or more of the build,
        // diff or test commands have changed the source file.
        //
        // Usually the problem is the build command, when you
        // changed things to generate a file which was previously
        // a source file, but you forgot to aerm the file.
        //
        if (c_src_data->file_fp)
        {
            string_ty       *absfn;
            int             ok;

            switch(c_src_data->action)
            {
            case file_action_remove:
            case file_action_transparent:
                break;

            case file_action_insulate:
                assert(0);
                break;

            case file_action_create:
            case file_action_modify:
#ifndef DEBUG
            default:
#endif
                absfn = change_file_path(cp, c_src_data->file_name);
                if (!absfn)
                    absfn = os_path_join(id, c_src_data->file_name);
                project_become(pp);
                ok = change_fingerprint_same(c_src_data->file_fp, absfn, 1);
                project_become_undo(pp);
                if (!ok)
                {
                    sub_context_ty      *scp;

                    scp = sub_context_new();
                    sub_var_set_string(scp, "File_Name", c_src_data->file_name);
                    change_error(cp, scp, i18n("build trashed $filename"));
                    sub_context_delete(scp);
                    ++nerr;
                }
                str_free(absfn);
                break;
            }
        }

        //
        // built files and removed file stop here
        //
        if (!transfer_architecture_times)
            continue;

        //
        // The edit number origin *must* be up-to-date, or the
        // change could not have got this far.
        //
        // As a branch advances, the edit field tracks the
        // history, but the edit_origin field is the number when
        // the file was first created or copied into the branch.
        // By definition, a file in a change is out of date when
        // it's edit_origin field does not equal the edit field
        // of its project.
        //
        // In order to merge branches, this must be done as a
        // cross branch merge in a change to that branch; the
        // edit_origin_new field of the change is copied
        // into the edit_origin field of the branch.
        //
        // p_src_data->edit
        //      The head revision of the branch.
        // p_src_data->edit_origin
        //      The version originally copied.
        //
        // c_src_data->edit
        //      Not meaningful until after integrate pass.
        // c_src_data->edit_origin
        //      The version originally copied.
        // c_src_data->edit_origin_new
        //      Updates branch edit_origin on
        //      integrate pass.
        //
        if (c_src_data->edit_origin_new)
        {
            assert(c_src_data->edit_origin_new->revision);
            if (p_src_data->edit_origin)
                history_version_type.free(p_src_data->edit_origin);
            p_src_data->edit_origin = c_src_data->edit_origin_new;
            c_src_data->edit_origin_new = 0;
        }

        //
        // update the test correlation
        //
	if (update_test_correlations)
	{
	    switch (c_src_data->usage)
	    {
	    case file_usage_config:
	    case file_usage_build:
	    case file_usage_test:
	    case file_usage_manual_test:
		break;

	    case file_usage_source:
		if (change_was_a_branch(cp))
		{
		    size_t          n;

		    if (!c_src_data->test)
			n = 0;
		    else
			n = c_src_data->test->length;
		    for (k = 0; k < n; ++k)
		    {
			string_ty       *fn;
			size_t          m;

			if (!p_src_data->test)
			{
			    p_src_data->test =
				(fstate_src_test_list_ty *)
				fstate_src_test_list_type.alloc();
			}
			fn = c_src_data->test->list[k];
			for (m = 0; m < p_src_data->test->length; ++m)
			    if (str_equal(p_src_data->test->list[m], fn))
				break;
			if (m >= p_src_data->test->length)
			{
			    meta_type *type_p = 0;
			    string_ty **addr_p =
				(string_ty **)
				fstate_src_test_list_type.list_parse
				(
				    p_src_data->test,
				    &type_p
				);
			    assert(type_p==&string_type);
			    *addr_p = str_copy(fn);
			}
		    }
		}
		else
		{
		    for (k = 0;; ++k)
		    {
			fstate_src_ty   *src2;
			size_t          m;

			src2 = change_file_nth(cp, k, view_path_first);
			if (!src2)
			    break;
			switch (src2->usage)
			{
			case file_usage_test:
			case file_usage_manual_test:
			    if (!p_src_data->test)
				p_src_data->test =
				    (fstate_src_test_list_ty *)
				    fstate_src_test_list_type.alloc();
			    for (m = 0; m < p_src_data->test->length; ++m)
			    {
				if
				(
				    str_equal
				    (
					p_src_data->test->list[m],
					src2->file_name
				    )
				)
				    break;
			    }
			    if (m >= p_src_data->test->length)
			    {
				meta_type *type_p = 0;
				string_ty **addr_p =
				    (string_ty **)
				    fstate_src_test_list_type.list_parse
				    (
					p_src_data->test,
					&type_p
				    );
				assert(type_p==&string_type);
				*addr_p = str_copy(src2->file_name);
			    }
			    break;

			case file_usage_source:
			case file_usage_config:
			case file_usage_build:
			    break;
			}
		    }
		}
		break;
	    }
	}
    }

    //
    // It is an error if the change has no current build.
    // It is an error if the change has no current test pass.
    // It is an error if the change has no current baseline test pass.
    // It is an error if the change has no current regression test pass.
    //
    if (change_build_required(cp) && !cstate_data->build_time)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Outstanding",
            change_outstanding_builds(cp, youngest)
        );
        sub_var_optional(scp, "Outstanding");
        change_error(cp, scp, i18n("bad ip, build required"));
        sub_context_delete(scp);
        ++nerr;
    }
    if (!cstate_data->test_exempt && !cstate_data->test_time)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Outstanding",
            change_outstanding_tests(cp, youngest)
        );
        sub_var_optional(scp, "Outstanding");
        change_error(cp, scp, i18n("bad ip, test required"));
        sub_context_delete(scp);
        ++nerr;
    }
    if (!cstate_data->test_baseline_exempt && !cstate_data->test_baseline_time)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Outstanding",
            change_outstanding_tests_baseline(cp, youngest)
        );
        sub_var_optional(scp, "Outstanding");
        change_error(cp, scp, i18n("bad ip, test -bl required"));
        ++nerr;
    }
    if
    (
        !cstate_data->regression_test_exempt
    &&
        !cstate_data->regression_test_time
    )
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar
        (
            scp,
            "Outstanding",
            change_outstanding_tests_regression(cp, youngest)
        );
        sub_var_optional(scp, "Outstanding");
        change_error(cp, scp, i18n("bad ip, test -reg required"));
        sub_context_delete(scp);
        ++nerr;
    }

    //
    // fail if any of the above detected errors
    //
    if (nerr)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_long(scp, "Number", nerr);
        sub_var_optional(scp, "Number");
        change_fatal(cp, scp, i18n("integrate pass fail"));
        sub_context_delete(scp);
    }

    //
    // transfer the build and test times into the project,
    // making sure to leave holes for architecture exemptions
    //
    p_cstate_data = pcp->cstate_get();
    p_cstate_data->build_time = cstate_data->build_time;
    p_cstate_data->test_time = cstate_data->test_time;
    p_cstate_data->test_baseline_time = cstate_data->test_baseline_time;
    p_cstate_data->regression_test_time = cstate_data->regression_test_time;
    if (p_cstate_data->architecture_times)
    {
        cstate_architecture_times_list_type.free
        (
            p_cstate_data->architecture_times
        );
    }
    p_cstate_data->architecture_times =
        (cstate_architecture_times_list_ty *)
        cstate_architecture_times_list_type.alloc();
    if (!cstate_data->architecture_times)
    {
	cstate_data->architecture_times =
	    (cstate_architecture_times_list_ty *)
	    cstate_architecture_times_list_type.alloc();
    }
    for (j = 0; j < cstate_data->architecture_times->length; ++j)
    {
        cstate_architecture_times_ty *catp;
        cstate_architecture_times_ty *patp;
        cstate_architecture_times_ty **addr;
        meta_type *type_p = 0;

        catp = cstate_data->architecture_times->list[j];
        addr =
            (cstate_architecture_times_ty **)
            cstate_architecture_times_list_type.list_parse
            (
                p_cstate_data->architecture_times,
                &type_p
            );
        assert(type_p == &cstate_architecture_times_type);
        patp =
            (cstate_architecture_times_ty *)
            cstate_architecture_times_type.alloc();
        *addr = patp;
        if (!catp->variant)
            this_is_a_bug();
        patp->variant = str_copy(catp->variant);
        if (!catp->node)
            this_is_a_bug();
        patp->node = str_copy(catp->node);
        patp->build_time = catp->build_time;
        patp->test_time = catp->test_time;
        patp->test_baseline_time = catp->test_baseline_time;
        patp->regression_test_time = catp->regression_test_time;
    }

    //
    // Make sure they aren't in a nuisance place.
    // (Many systems can't delete a directory if any process has
    // its current directory within it.)
    //
    os_become_orig();
    cwd = os_curdir();
    os_become_undo();
    if (os_below_dir(change_development_directory_get(cp, 1), cwd))
        change_fatal(cp, 0, i18n("leave dev dir"));
    if (os_below_dir(id, cwd))
        change_fatal(cp, 0, i18n("leave int dir"));
    if (os_below_dir(pp->baseline_path_get(true), cwd))
        project_fatal(pp, 0, i18n("leave baseline"));

    //
    // merge copyright years
    //
    project_copyright_years_merge(pp, cp);

    //
    // note that the project has no current integration
    //
    project_current_integration_set(pp, 0);

    //
    // Clear the default-change field of the user row.
    // Remove the change from the list of assigned changes in the user
    // change table (in the user row).
    //
    up->own_remove(pp, change_number);

    //
    // Add all of the generated (build) files in the project to this
    // change so that their history is remembered.
    //
    for (j = 0;; ++j)
    {
        fstate_src_ty   *c_src_data;
        fstate_src_ty   *p_src_data;

        p_src_data = pp->file_nth(j, view_path_extreme);
        if (!p_src_data)
            break;
        switch (p_src_data->usage)
        {
        case file_usage_build:
            //
            // If the built project file is already present in the
            // change set, don't do anything.
            //
            c_src_data = change_file_find(cp, p_src_data, view_path_first);
            if (c_src_data)
                continue;

            //
            // If the file is unchanged, don't do anything.
            //
            {
                nstring p1
                    (
                        os_path_join
                        (
                            change_integration_directory_get(cp, 1),
                            p_src_data->file_name
                        )
                    );
                assert(!p1.empty());
                nstring p2(project_file_path(pp, p_src_data));
                assert(!p2.empty());
                user_ty::become scoped(up);
                // make sure the file is there, ignore it if it's not
                if (!os_exists(p1))
                    continue;
                if (!os_exists(p2))
                    continue;
                if (!files_are_different(p1, p2))
                    continue;
            }

            c_src_data = cp->file_new(p_src_data);
            assert(p_src_data->edit);
            assert(p_src_data->edit->revision);
            c_src_data->edit_origin = history_version_copy(p_src_data->edit);
            c_src_data->action = file_action_modify;

            //
            // Make sure the branch has them, too, otherwise the code will
            // barf a little further on.
            //
            project_file_shallow(pp, c_src_data->file_name, change_number);
            break;

        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;
        }
    }

    //
    // Update the edit history of each changed file.
    // Update the edit number of each file.
    // Remove the fingerprints for each file.
    // Unlock each file.
    //
    log_open(change_logfile_get(cp), pup, log_style);
    string_list_ty trashed;
    change_run_history_transaction_begin_command(cp);
    quit_action_history_transaction_abort aborter(cp);
    quit_register(aborter);
    for (j = 0;; ++j)
    {
        fstate_src_ty   *c_src_data;
        fstate_src_ty   *p_src_data;
        string_ty       *absfn;

        c_src_data = change_file_nth(cp, j, view_path_first);
        if (!c_src_data)
            break;
        if (c_src_data->file_fp)
        {
            fingerprint_type.free(c_src_data->file_fp);
            c_src_data->file_fp = 0;
        }
        if (c_src_data->diff_file_fp)
        {
            fingerprint_type.free(c_src_data->diff_file_fp);
            c_src_data->diff_file_fp = 0;
        }
        if (c_src_data->idiff_file_fp)
        {
            fingerprint_type.free(c_src_data->idiff_file_fp);
            c_src_data->idiff_file_fp = 0;
        }
        if (c_src_data->architecture_times)
        {
            fstate_src_architecture_times_list_type.free
            (
                c_src_data->architecture_times
            );
            c_src_data->architecture_times = 0;
        }

        //
        // Do absolutely nothing for transparent branch files.
        //
        switch (c_src_data->action)
        {
        case file_action_transparent:
            if (change_was_a_branch(cp))
                continue;
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_remove:
            break;

        case file_action_insulate:
            assert(0);
            break;
        }

        p_src_data =
            change_file_find(pcp, c_src_data->file_name, view_path_first);
        if (!p_src_data)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_charstar(scp, "File_Name", "fstate file");
            sub_var_set_format
            (
                scp,
                "FieLD_Name",
                "src[%s]",
                c_src_data->file_name->str_text
            );
            project_fatal
            (
                pp,
                scp,
                i18n("$filename: corrupted \"$field_name\" field")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
        p_src_data->locked_by = 0;
        p_src_data->about_to_be_copied_by = 0;
        int read_only_mode = 0444;
        switch (c_src_data->action)
        {
        case file_action_create:
            //
            // Because history is never thrown away,
            // we could be reusing an existing history file.
            // Also, branches already have history when the
            // files they created are integrated.
            //
            if (p_src_data->deleted_by)
            {
                p_src_data->deleted_by = 0;
                goto reusing_an_old_file;
            }

            //
            // Remember the last-modified-time, so we can
            // restore it if the history tool messes with it.
            // Remember whether the file is executable.
            // (It is allowed to change with every edit.)
            //
            absfn = change_file_path(cp, c_src_data->file_name);
            if (!absfn)
                absfn = os_path_join(id, c_src_data->file_name);
            project_become(pp);
            mtime = os_mtime_actual(absfn);
            p_src_data->executable = os_executable(absfn);
            c_src_data->executable = p_src_data->executable;
            project_become_undo(pp);

            read_only_mode = 0444;
            if (c_src_data->executable)
                read_only_mode |= 0111;
            read_only_mode &= ~change_umask(cp);

            //
            // create the history
            //
            change_run_history_create_command(cp, c_src_data);

            //
            // Update the head revision number.
            // (Create also sets edit_origin field,
            // if not already set)
            //
            if (p_src_data->edit)
                history_version_type.free(p_src_data->edit);
            p_src_data->edit = history_version_copy(c_src_data->edit);
            if (!p_src_data->edit_origin)
                p_src_data->edit_origin =
                    history_version_copy(p_src_data->edit);

            //
            // Some history commands change the mode of the file, in
            // particular making read-only source files read-write,
            // which is a Bad Thing in the baseline.
            //
            project_become(pp);
            os_chmod(absfn, read_only_mode);

            //
            // Set the last-modified-time, just in case the
            // history tool changed it, even if it didn't
            // change the file content.  This reduces the
            // build burden imposed by an integration.
            //
            // We do this before we check the fingerprint.
            // This has the side-effect of forcing the
            // fingerprint, but it also gets the fingerprint
            // right in the project file's attributes.
            //
            os_mtime_set_errok(absfn, mtime);

            //
            // Many history tools (e.g. RCS) can modify the
            // contents of the file when it is committed.
            // While there are usually options to turn this
            // off, they are seldom used.  The problem is:
            // if the commit changes the file, the source
            // in the repository now no longer matches the
            // object file in the repository - i.e. the
            // history tool has compromised the referential
            // integrity of the repository.
            //
            // Keep track of them, we will generate an
            // error message after all of the commands have
            // been run.
            //
            if (p_src_data->file_fp)
            {
                assert(p_src_data->file_fp->youngest>=0);
                assert(p_src_data->file_fp->oldest>=0);
                if (!change_fingerprint_same(p_src_data->file_fp, absfn, 1))
                {
                    trashed.push_back(c_src_data->file_name);
                }
                assert(p_src_data->file_fp->youngest>0);
                assert(p_src_data->file_fp->oldest>0);
            }
            str_free(absfn);
            project_become_undo(pp);
            break;

        case file_action_modify:
            reusing_an_old_file:
            trace(("modify\n"));

            if (p_src_data->edit)
            {
                history_version_type.free(p_src_data->edit);
                p_src_data->edit = 0;
            }
            if (c_src_data->edit)
            {
                history_version_type.free(c_src_data->edit);
                c_src_data->edit = 0;
            }

            //
            // 1. Remember the last-modified-time, so we can
            // restore it if the history tool messes with it.
            // 2. Remember whether the file is executable.  The
            // executable flag must be saved before updating the
            // history because some setup (the RCS setup in the
            // manual) cause the history tool to modify the file's
            // permissions.
            // (It is allowed to change with every edit.)
            //
            absfn = change_file_path(cp, c_src_data->file_name);
            if (!absfn)
                absfn = os_path_join(id, c_src_data->file_name);
            project_become(pp);
            mtime = os_mtime_actual(absfn);
            p_src_data->executable = os_executable(absfn);
            c_src_data->executable = p_src_data->executable;
            project_become_undo(pp);

            read_only_mode = 0444;
            if (c_src_data->executable)
                read_only_mode |= 0111;
            read_only_mode &= ~change_umask(cp);

            //
            // update the history
            //
            change_run_history_put_command(cp, c_src_data);

            //
            // Update the head revision number.
            // (Leave edit_origin field alone.)
            //
            if (!p_src_data->edit_origin && p_src_data->edit)
                p_src_data->edit_origin =
                    history_version_copy(p_src_data->edit);
            p_src_data->edit = history_version_copy(c_src_data->edit);
            if (!p_src_data->edit_origin)
                p_src_data->edit_origin =
                    history_version_copy(p_src_data->edit);

            //
            // Some history commands change the mode of the file, in
            // particular making read-only source files read-write,
            // which is a Bad Thing in the baseline.
            //
            project_become(pp);
            os_chmod(absfn, read_only_mode);

            //
            // Set the last-modified-time, just in case the
            // history tool changed it, even if it didn't
            // change the file content.  This reduces the
            // build burden imposed by an integration.
            //
            // We do this before we check the fingerprint.
            // This has the side-effect of forcing the
            // fingerprint, but it also gets the fingerprint
            // right in the project file's attributes.
            //
            os_mtime_set_errok(absfn, mtime);

            //
            // Many history tools (e.g. RCS) can modify the
            // contents of the file when it is committed.
            // While there are usually options to turn this
            // off, they are seldom used.  The problem is:
            // if the commit changes the file, the source
            // in the repository now no longer matches the
            // object file in the repository - i.e. the
            // history tool has compromised the referential
            // integrity of the repository.
            //
            // Keep track of them, we will generate an
            // error message after all of the commands have
            // been run.
            //
            if (p_src_data->file_fp)
            {
                if (!change_fingerprint_same(p_src_data->file_fp, absfn, 1))
                {
                    trashed.push_back(c_src_data->file_name);
                }
                assert(p_src_data->file_fp->youngest>0);
                assert(p_src_data->file_fp->oldest>0);
            }
            str_free(absfn);
            project_become_undo(pp);
            break;

        case file_action_remove:
            //
            // don't need to do much for deletions.
            // Note that we never throw the history file away.
            //
            trace(("remove\n"));
            p_src_data->deleted_by = change_number;
            break;

        case file_action_insulate:
            //
            // This should never happen: aede will fail if
            // there are any insulation files.
            //
            assert(0);
            break;

        case file_action_transparent:
            trace(("transparent\n"));
            break;
        }

        //
        // The file exists, now, so clear the "about to be
        // created" flag.
        //
        // This flag can require clearing even if the file action
        // isn't "create", for various branch manipulations of
        // files created on the branch.  Clearing it redundantly
        // is harmless.
        //
        p_src_data->about_to_be_created_by = 0;

        //
        // make sure the branch action is appropriate
        //
        switch (c_src_data->action)
        {
        case file_action_remove:
            //
            // If the action is a real remove (no move field present)
            // we must reset the project idea of the old name in order
            // to avoid loops.  If the action is a rename (move field
            // present) the project idea of the old name must be
            // updated with the value from the change.
            //
            p_src_data->action = file_action_remove;
            if (p_src_data->move)
            {
                str_free(p_src_data->move);
                p_src_data->move = 0;
            }
            if (c_src_data->move)
                p_src_data->move = str_copy(c_src_data->move);
            break;

        case file_action_transparent:
            //
            // The move field is supposed to not be present in a
            // transparent action.
            //
            p_src_data->action = file_action_transparent;
            if (p_src_data->move)
            {
                str_free(p_src_data->move);
                p_src_data->move = 0;
            }
            break;

        case file_action_create:
        case file_action_modify:
        case file_action_insulate:
            if (!pp->is_a_trunk())
            {
                fstate_src_ty   *pp_src_data;

                pp_src_data =
                    project_file_find
                    (
                        pp->parent_get(),
                        c_src_data->file_name,
                        view_path_none
                    );
                if
                (
                    pp_src_data
                &&
                    !pp_src_data->about_to_be_created_by
                &&
                    !pp_src_data->deleted_by
                )
                {
                    //
                    // The move field is supposed to not be present in
                    // a modify operation.
                    //
                    p_src_data->action = file_action_modify;
                    if (p_src_data->move)
                    {
                        str_free(p_src_data->move);
                        p_src_data->move = 0;
                    }
                }
                else
                {
                    //
                    // If the action is a rename (move field present)
                    // then we must update the project idea of the old
                    // name with the value from the change.
                    // In other cases we must preserve the move value
                    // in the project to not break an existing rename,
                    // if any.
                    //
                    p_src_data->action = file_action_create;
                    if (c_src_data->move)
                    {
                        if (p_src_data->move)
                            str_free(p_src_data->move);
                        p_src_data->move = str_copy(c_src_data->move);
                    }
                }
            }
            else
            {
                //
                // If the action is a rename (move field present)
                // then we must update the project idea of the old
                // name with the value from the change.
                // In other cases we must preserve the move value
                // in the project to not break an existing rename,
                // if any.
                //
                p_src_data->action = file_action_create;
                if (c_src_data->move)
                {
                    if (p_src_data->move)
                        str_free(p_src_data->move);
                    p_src_data->move = str_copy(c_src_data->move);
                }
            }
            break;
        }

        //
        // For the project trunk, the edit number and the edit
        // number origin are always identical.  Otherwise, when
        // branches inherit the project files of their parents,
        // the parent's files will appear to be out of date,
        // even though they can not be.
        //
        if (pp->is_a_trunk())
        {
            assert(p_src_data->edit);
            assert(p_src_data->edit->revision);
            assert(p_src_data->edit_origin);
            assert(p_src_data->edit_origin->revision);
            if
            (
                !str_equal
                (
                    p_src_data->edit->revision,
                    p_src_data->edit_origin->revision
                )
            )
            {
                history_version_type.free(p_src_data->edit_origin);
                p_src_data->edit_origin =
                    history_version_copy(p_src_data->edit);
            }
        }
    }

    //
    // Unlock the files removed because unchanged.
    //
    for (size_t i = 0; i < removed_file.size(); ++i)
    {
        fstate_src_ty   *p_src_data =
            change_file_find(pcp, removed_file[i], view_path_first);
        if (!p_src_data)
            continue;

        p_src_data->locked_by = 0;
    }
    quit_unregister(aborter);
    change_run_history_transaction_end_command(cp);

    if (pconf_data->history_label_command)
    {
        //
        // The above code changed the project file list,
        // so we need to clear out the cache before the following will work.
        //
        pp->file_list_invalidate();

        //
        // For all the project's files, label the history.
        //
        // Note that at this point in the aeipass commant, our
        // internal project file list includes all of the changes
        // madeby the change.
        //
        for (j = 0;; ++j)
        {
            fstate_src_ty   *src_data;

            src_data = cp->pp->file_nth(j, view_path_extreme);
            if (!src_data)
                break;
            change_run_history_label_command
            (
                cp,
                src_data,
                project_version_get(cp->pp)
            );
        }
    }

    //
    // Many history tools (e.g. RCS) can modify the contents of the
    // file when it is committed.  While there are usually options to
    // turn this off, they are seldom used.  The problem is: if the
    // commit changes the file, the source in the repository now no
    // longer matches the object file in the repository - i.e. the
    // history tool has compromised the referential integrity of
    // the repository.
    //
    // Check here if this is the case, and emit an error message if
    // so.  (It could be a fatal error, just a warning, or completely
    // ignored, depending on the history_put_trashes_file field of
    // the project config file.
    //
    change_history_trashed_fingerprints(cp, &trashed);

    //
    // Advance the change to the 'completed' state.
    // Clear the build-time field.
    // Clear the test-time field.
    // Clear the test-baseline-time field.
    //
    cstate_data->state = cstate_state_completed;
    change_build_times_clear(cp);
    dev_dir = str_copy(change_top_path_get(cp, 1));
    change_development_directory_clear(cp);
    new_baseline = str_copy(change_integration_directory_get(cp, 1));
    id = str_copy(id);
    change_integration_directory_clear(cp);
    dev_name = change_developer_name(cp);

    //
    // Need to get rid of the old symlinks for old integration directories.
    // But only if it is aeib -minimum situation (maybe implicit).
    //
    if (cstate_data->minimum_integration)
    {
        string_ty       *dir;

        dir = project_top_path_get(pp, 0);
        user_ty::become scoped(pp->get_user());
        commit_unlink_symlinks(dir);
    }

    //
    // we are going to keep the delta,
    // and throw away the old baseline
    //
    old_baseline =
        str_format
        (
            "%s.D%3.3ld",
            pp->baseline_path_get(true)->str_text,
            cstate_data->delta_number - 1
        );
    new_baseline = change_integration_directory_get(cp, 1);
    project_become(pp);
    commit_rename(pp->baseline_path_get(true), old_baseline);
    commit_rename(new_baseline, pp->baseline_path_get(true));
    commit_symlink(str_from_c("baseline"), new_baseline); // relative!
    if (os_unthrottle())
        commit_rmdir_tree_errok(old_baseline);
    else
        commit_rmdir_tree_bg(old_baseline);
    project_become_undo(pp);
    str_free(old_baseline);

    //
    // throw away the development directory
    //
    if (change_was_a_branch(cp))
    {
        user_ty::become scoped(pp->get_user());
        if (os_unthrottle())
            commit_rmdir_tree_errok(dev_dir);
        else
            commit_rmdir_tree_bg(dev_dir);
    }
    else
    {
        devup = user_ty::create(nstring(dev_name));
        user_ty::become scoped(devup);
        if (os_unthrottle())
            commit_rmdir_tree_errok(dev_dir);
        else
            commit_rmdir_tree_bg(dev_dir);
    }
    str_free(dev_dir);

    tml.list = 0;
    tml.len = 0;
    tml.max_len = 0;
    if (pconf_data->build_time_adjust != pconf_build_time_adjust_dont_adjust)
    {
        //
        // Collect the modify times of all the files in the integration
        // directory.  Sort the file times in ascending order, and then
        // renumber from when we got the lock.
        //
        change_verbose(cp, 0, i18n("adjust file modification times"));
        project_become(pp);
        dir_walk(id, time_map_get, &tml);

        //
        // This is not a valid assertion because a change could remove a
        // file which has no ancestors, and thus not affect any other
        // file's modification time.
        //
        // if (tml.len < 1)
        //          this_is_a_bug();
        //
#if MTIME_BLURB
        if (tml.len > 0)
        {
            char b1[30];
            strendcpy(b1, ctime(&tml.list[0].old), b1 + sizeof(b1));
            char b2[30];
            strendcpy(b2, ctime(&tml.list[tml.len - 1].old), b2 + sizeof(b2));
            error_raw
            (
                "original times range from %.24s to %.24s = %d seconds",
                b1,
                b2,
                (int)(1 + tml.list[tml.len - 1].old - tml.list[0].old)
            );
        }
#endif
        for (j = 0; j < tml.len; ++j)
        {
            assert(tml.list[j].old>=tml.time_aeib);
            assert(j==0||tml.list[j].old>tml.list[j-1].old);
            tml.list[j].becomes = tml.time_aeip + j;
        }
#if MTIME_BLURB
        if (tml.len > 0)
        {
            char b1[30];
            strendcpy(b1, ctime(&tml.list[0].becomes), b1 + sizeof(b1));
            char b2[30];
            strendcpy(b2, ctime(&tml.list[tml.len-1].becomes), b2 + sizeof(b2));
            error_raw
            (
                "adjusted times range from %.24s to %.24s = %d seconds",
                b1,
                b2,
                (int)(1 + tml.list[tml.len - 1].becomes - tml.list[0].becomes)
            );
        }
#endif
        dir_walk(id, time_map_set, &tml);
        project_become_undo(pp);

        //
        // Give the user a chance to re-sync any database associated
        // with the build tool.  (This is different to the integrate
        // pass notify command.)
        //
        change_run_build_time_adjust_notify_command(cp);
    }
    str_free(id);
    id = 0;

    //
    // Deal with mod times which extend into the future.
    //
    switch (pconf_data->build_time_adjust)
    {
    case pconf_build_time_adjust_dont_adjust:
        break;

    case pconf_build_time_adjust_adjust_and_sleep:
        if (tml.len > 0)
        {
            for (;;)
            {
                now_clear();
                time_t time_final = now();

                //
                // We have to add a second here because if there is
                // a script which is blocked waiting on the baseline
                // lock, it is possible for the aeipass to complete and
                // the script to copy files (etc) and build ALL WITHIN
                // THE ONE SECOND.  This means that time stamps for
                // that second can be broken.  This is particularly
                // evident in Aegis' own test scripts, however the
                // problem is more general.  The solution is to add a
                // single second, so that any post-aeipass activity is
                // guaranteed to be in a later second than the baseline
                // file stamps.
                //
                long nsec = tml.list[tml.len - 1].becomes - time_final + 1;
                if (nsec <= 0)
                    break;

                sub_context_ty sc;
                sc.var_set_long("Number", nsec);
                sc.error_intl(i18n("throttling $number seconds"));
                sleep(nsec);
            }
        }
        break;

    case pconf_build_time_adjust_adjust_only:
        //
        // warn the user if some files have been timed into the future
        //
        if (tml.len > 0)
        {
            time_t time_final = now();

            //
            // We don't add one here (see above comment) because it is a
            // statement about the time stamps, not a calculation of how
            // long to sleep.
            //
            long nsec = tml.list[tml.len - 1].becomes - time_final;
            if (nsec > 0)
            {
                sub_context_ty sc;
                sc.var_set_long("Number", nsec);
                sc.var_optional("Number");
                sc.error_intl(i18n("warning: file times in future"));
            }
        }
        break;
    }

    //
    // Write the change table row.
    // Write the user table row.
    // Release advisory locks.
    //
    change_cstate_write(cp);
    up->ustate_write();
    pp->pstate_write();
    change_pfs_write(cp); // must be after project_pstate_write
    change_verbose(cp, 0, i18n("discard old directories"));
    commit();
    lock_release();

    //
    // run the notify command
    //
    // Note that we set the AEGIS_INTEGRATION_DIRECTORY environment
    // variable further back in the code.
    //
    cp->run_integrate_pass_notify_command();

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("integrate pass complete"));
    change_free(cp);
    project_free(pp);

    //
    // Give the tests time to finish removing the old baseline and
    // the old development directory.
    //
    os_throttle();
    trace(("}\n"));
}


void
integrate_pass(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, integrate_pass_help, 0 },
        { arglex_token_list, integrate_pass_list, 0 },
    };

    trace(("integrate_pass()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_pass_main);
    trace(("}\n"));
}
