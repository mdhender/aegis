//
//	aegis - project change supervisor
//	Copyright (C) 1991-2004 Peter Miller;
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
// MANIFEST: functions for implementing integrate pass
//

#include <ac/errno.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ac/unistd.h>

#include <aeip.h>
#include <ael/change/by_state.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <commit.h>
#include <change/branch.h>
#include <change/file.h>
#include <dir.h>
#include <env.h>
#include <error.h>
#include <file.h>
#include <glue.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <metrics.h>
#include <mem.h>
#include <now.h>
#include <progname.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <uuidentifier.h>

#define MTIME_BLURB 0

typedef struct time_map_ty time_map_ty;
struct time_map_ty
{
    time_t	    old;
    time_t	    becomes;
};

typedef struct time_map_list_ty time_map_list_ty;
struct time_map_list_ty
{
    time_map_ty	    *list;
    size_t	    len;
    size_t	    max;
    time_t	    time_aeib;
    time_t	    time_aeip;
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
    string_ty	    *project_name;

    trace(("integrate_pass_list()\n{\n"));
    arglex();
    project_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(integrate_pass_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    // fall through...

	case arglex_token_string:
	    arglex_parse_project(&project_name, integrate_pass_usage);
	    continue;
	}
	arglex();
    }
    list_changes_in_state_mask
    (
	project_name,
	1 << cstate_state_being_integrated
    );
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


static void
time_map_get(void *p, dir_walk_message_ty message, string_ty *path,
    struct stat *st)
{
    time_map_list_ty *tlp;
    time_map_ty	    *tp;
    time_t	    t;
    long	    min;
    long	    max;
    long	    j;

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
    min = 0;
    max = (long)tlp->len - 1;
    while (min <= max)
    {
	long		mid;
	time_t		mid_t;

	mid = (min + max) / 2;
	mid_t = tlp->list[mid].old;
	if (mid_t == t)
	    return;
	if (mid_t < t)
	    min = mid + 1;
	else
	    max = mid - 1;
    }

    //
    // This is a new time, insert it into the list sorted by time.
    //
    if (tlp->len >= tlp->max)
    {
	tlp->max += 100;
	tlp->list =
	    (time_map_ty *)
            mem_change_size(tlp->list, tlp->max * sizeof(tlp->list[0]));
    }
    for (j = tlp->len; j > min; --j)
	tlp->list[j] = tlp->list[j - 1];
    tlp->len++;
    tp = &tlp->list[min];
    tp->old = st->st_mtime;
    tp->becomes = st->st_mtime;
}


static void
time_map_set(void *p, dir_walk_message_ty message, string_ty *path,
    struct stat *st)
{
    time_map_list_ty *tlp;
    time_t	    t;
    long	    min;
    long	    max;

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
    min = 0;
    max = (long)tlp->len - 1;
    while (min <= max)
    {
	long		mid;
	time_t		mid_t;

	mid = (min + max) / 2;
	mid_t = tlp->list[mid].old;
	if (mid_t == t)
	{
	    min = mid;
	    max = mid;
	    break;
	}
	if (mid_t < t)
	    min = mid + 1;
	else
	    max = mid - 1;
    }

    //
    // If the file time has been altered since aeipass began (and
    // thus is not in the list), which hopefully is *very* rare, as
    // it requires direct human interference, use a close time.
    //
    if (min >= (long)tlp->len)
    {
	min = (long)tlp->len - 1;
    }

    //
    // set the file time
    //
    os_mtime_set_errok(path, tlp->list[min].becomes);
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
	sub_context_ty	*scp;
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
	    sub_context_ty	*scp;
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
    string_list_destructor(&wl);
    trace(("}\n"));
#endif
}


static void
integrate_pass_main(void)
{
    time_t	    mtime;
    time_t	    youngest;
    string_ty	    *hp;
    string_ty	    *id;
    string_ty	    *cwd;
    cstate_ty	    *cstate_data;
    string_ty	    *old_baseline;
    string_ty	    *new_baseline;
    string_ty	    *dev_dir;
    string_ty	    *int_name;
    string_ty	    *rev_name;
    string_ty	    *dev_name;
    cstate_history_ty *history_data;
    size_t	    j;
    size_t	    k;
    int		    ncmds;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    user_ty	    *devup;
    user_ty	    *pup;
    int		    nerr;
    int		    diff_whine;
    change_ty	    *pcp;
    cstate_ty	    *p_cstate_data;
    time_map_list_ty tml;
    time_t	    time_final;
    string_list_ty  trashed;
    pconf_ty        *pconf_data;

    trace(("integrate_pass_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    log_style = log_style_append_default;
    nerr = 0;
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
	    user_lock_wait_argument(integrate_pass_usage);
	    break;
	}
	arglex();
    }

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate user data
    //
    up = user_executing(pp);
    pup = project_user(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
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
    project_pstate_lock_prepare(pp);
    change_cstate_lock_prepare(cp);
    user_ustate_lock_prepare(up);
    project_baseline_write_lock_prepare(pp);
    project_history_lock_prepare(pp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    //
    // It is an error if the change is not in the being_integrated state.
    // It is an error if the change is not assigned to the current user.
    //
    if (cstate_data->state != cstate_state_being_integrated)
	change_fatal(cp, 0, i18n("bad ip state"));
    if (!str_equal(change_integrator_name(cp), user_name(up)))
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
    // Walk the change files, making sure
    //	    1. the change has been diffed (except for the lowest b.l.)
    //	    2. parent files are copied into the change
    //	    3. test times are transferred
    //	    4. The fingerprint is still correct.
    //
    pcp = project_change_get(pp);
    diff_whine = 0;
    for (j = 0;; ++j)
    {
	fstate_src_ty   *c_src_data;
	fstate_src_ty   *p_src_data;
	int		transfer_architecture_times;
	int		transfer_file_times;
	int		transfer_diff_file_times;

	c_src_data = change_file_nth(cp, j, view_path_first);
	if (!c_src_data)
	    break;
	trace(("file_name = \"%s\"\n", c_src_data->file_name->str_text));

	//
	// check the the file has been differenced
	//
	if (!diff_whine)
	{
	    switch (c_src_data->action)
	    {
	    case file_action_remove:
		if
		(
		    c_src_data->move
		&&
		    change_file_find(cp, c_src_data->move, view_path_first)
		)
		    break;
		// fall through...

	    case file_action_create:
	    case file_action_modify:
#ifndef DEBUG
	    default:
#endif
		if
		(
		    pp->parent
		&&
		    c_src_data->usage != file_usage_build
		&&
		    (
			!c_src_data->idiff_file_fp
		    ||
			!c_src_data->idiff_file_fp->youngest
		    )
		)
		{
		    change_error(cp, 0, i18n("diff required"));
		    ++nerr;
		    ++diff_whine;
		}
		break;

	    case file_action_insulate:
	    case file_action_transparent:
		break;
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
	if (!c_src_data->uuid && c_src_data->action == file_action_create)
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
	// Preserve the file movement information (this makes
	// branches and changes more symmetric, as well as
	// preserving useful information).
	//
	if (c_src_data->move)
	{
	    if (p_src_data->move)
		str_free(p_src_data->move);
	    p_src_data->move = str_copy(c_src_data->move);
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
		type_ty		*type_p;

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
	    // when aegis was upgraded from 2.3 to 3.0.	 Give an
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
	    string_ty	    *absfn;
	    int		    ok;

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
		project_become_undo();
		if (!ok)
		{
		    sub_context_ty	*scp;

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
	//	The head revision of the branch.
	// p_src_data->edit_origin
	//	The version originally copied.
	//
	// c_src_data->edit
	//	Not meaningful until after integrate pass.
	// c_src_data->edit_origin
	//	The version originally copied.
	// c_src_data->edit_origin_new
	//	Updates branch edit_origin on
	//	integrate pass.
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
		size_t		n;

		if (!c_src_data->test)
		    n = 0;
		else
		    n = c_src_data->test->length;
		for (k = 0; k < n; ++k)
		{
		    string_ty	    *fn;
		    size_t	    m;

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
			string_ty	**addr_p;
			type_ty		*type_p;

			addr_p =
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
		    size_t	    m;

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
			    string_ty       **addr_p;
			    type_ty         *type_p;

			    addr_p =
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

    //
    // It is an error if the change has no current build.
    // It is an error if the change has no current test pass.
    // It is an error if the change has no current baseline test pass.
    // It is an error if the change has no current regression test pass.
    //
    if (!cstate_data->build_time)
    {
	sub_context_ty	*scp;

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
	sub_context_ty	*scp;

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
	sub_context_ty	*scp;

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
	sub_context_ty	*scp;

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
	sub_context_ty	*scp;

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
    p_cstate_data = change_cstate_get(pcp);
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
	this_is_a_bug();
    for (j = 0; j < cstate_data->architecture_times->length; ++j)
    {
	cstate_architecture_times_ty *catp;
	cstate_architecture_times_ty *patp;
	cstate_architecture_times_ty **addr;
	type_ty		*type_p;

	catp = cstate_data->architecture_times->list[j];
	addr =
	    (cstate_architecture_times_ty **)
	    cstate_architecture_times_list_type.list_parse
	    (
		p_cstate_data->architecture_times,
		&type_p
	    );
	assert(type_p==&cstate_architecture_times_type);
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
    if (os_below_dir(project_baseline_path_get(pp, 1), cwd))
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
    user_own_remove(up, project_name_get(pp), change_number);

    //
    // Add all of the generated (build) files in the project to this
    // change so that their history is remembered.
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *c_src_data;
	fstate_src_ty   *p_src_data;

	p_src_data = project_file_nth(pp, j, view_path_extreme);
	if (!p_src_data)
	    break;
	switch (p_src_data->usage)
	{
	case file_usage_build:
	    c_src_data =
		change_file_find(cp, p_src_data->file_name, view_path_first);
	    if (c_src_data)
		continue;
	    c_src_data = change_file_new(cp, p_src_data->file_name);
	    assert(p_src_data->edit);
	    assert(p_src_data->edit->revision);
	    c_src_data->edit_origin = history_version_copy(p_src_data->edit);
	    c_src_data->action = file_action_modify;
	    change_file_copy_basic_attributes(c_src_data, p_src_data);

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
    ncmds = 0;
    hp = project_history_path_get(pp);
    string_list_constructor(&trashed);
    for (j = 0;; ++j)
    {
	fstate_src_ty   *c_src_data;
	fstate_src_ty   *p_src_data;
	string_ty	*absfn;

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
	    //
	    absfn = change_file_path(cp, c_src_data->file_name);
	    if (!absfn)
		absfn = os_path_join(id, c_src_data->file_name);
	    project_become(pp);
	    mtime = os_mtime_actual(absfn);
	    project_become_undo();

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
	    // Remember whether the file is executable.
	    // (It is allowed to change with every edit.)
	    //
	    project_become(pp);
	    p_src_data->executable = (boolean_ty)os_executable(absfn);
	    c_src_data->executable = (boolean_ty)p_src_data->executable;

	    //
	    // Set the last-modified-time, just in case the
	    // history tool changed it, even if it didn't
	    // change the file content.	 This reduces the
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
		    string_list_append(&trashed, c_src_data->file_name);
		}
		assert(p_src_data->file_fp->youngest>0);
		assert(p_src_data->file_fp->oldest>0);
	    }
	    str_free(absfn);
	    project_become_undo();
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
	    // Remember the last-modified-time, so we can
	    // restore it if the history tool messes with it.
	    //
	    absfn = change_file_path(cp, c_src_data->file_name);
	    if (!absfn)
		absfn = os_path_join(id, c_src_data->file_name);
	    project_become(pp);
	    mtime = os_mtime_actual(absfn);
	    project_become_undo();

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
	    // Remember whether the file is executable.
	    // (It is allowed to change with every edit.)
	    //
	    project_become(pp);
	    p_src_data->executable = (boolean_ty)os_executable(absfn);
	    c_src_data->executable = p_src_data->executable;
	    project_become_undo();

	    //
	    // Set the last-modified-time, just in case the
	    // history tool changed it, even if it didn't
	    // change the file content.	 This reduces the
	    // build burden imposed by an integration.
	    //
	    // We do this before we check the fingerprint.
	    // This has the side-effect of forcing the
	    // fingerprint, but it also gets the fingerprint
	    // right in the project file's attributes.
	    //
	    project_become(pp);
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
		    string_list_append(&trashed, c_src_data->file_name);
		}
		assert(p_src_data->file_fp->youngest>0);
		assert(p_src_data->file_fp->oldest>0);
	    }
	    str_free(absfn);
	    project_become_undo();
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
	// files created on the branch.	 Clearing it redundantly
	// is harmless.
	//
	p_src_data->about_to_be_created_by = 0;

	//
	// make sure the branch action is appropriate
	//
	switch (c_src_data->action)
	{
	case file_action_remove:
	    p_src_data->action = file_action_remove;
	    break;

	case file_action_transparent:
	    p_src_data->action = file_action_transparent;
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    if (pp->parent)
	    {
		fstate_src_ty   *pp_src_data;

		pp_src_data =
		    project_file_find
		    (
			pp->parent,
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
		    p_src_data->action = file_action_modify;
		else
		    p_src_data->action = file_action_create;
	    }
	    else
		p_src_data->action = file_action_create;
	    break;
	}

	//
	// For the project trunk, the edit number and the edit
	// number origin are always identical.	Otherwise, when
	// branches inherit the project files of their parents,
	// the parent's files will appear to be out of date,
	// even though they can not be.
	//
	if (!pp->parent)
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

    pconf_data = change_pconf_get(cp, 0);
    if (pconf_data->history_label_command)
    {
	//
	// The above code changed the project file list,
	// so we need to clear out the cache before the following will work.
	//
	project_file_list_invalidate(pp);

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

	    src_data = project_file_nth(cp->pp, j, view_path_extreme);
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
    string_list_destructor(&trashed);

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
    int_name = change_integrator_name(cp);
    rev_name = change_reviewer_name(cp);
    dev_name = change_developer_name(cp);

    //
    // Need to get rid of the old symlinks for old integration directories.
    // But only if it is aeib -minimum situation (maybe implicit).
    //
    if (cstate_data->minimum_integration)
    {
	string_ty       *dir;

	dir = project_top_path_get(pp, 0);
	project_become(pp);
	commit_unlink_symlinks(dir);
	project_become_undo();
    }

    //
    // we are going to keep the delta,
    // and throw away the old baseline
    //
    old_baseline =
	str_format
	(
	    "%s.D%3.3ld",
	    project_baseline_path_get(pp, 1)->str_text,
	    cstate_data->delta_number - 1
	);
    new_baseline = change_integration_directory_get(cp, 1);
    project_become(pp);
    commit_rename(project_baseline_path_get(pp, 1), old_baseline);
    commit_rename(new_baseline, project_baseline_path_get(pp, 1));
    commit_symlink(str_from_c("baseline"), new_baseline); // relative!
    commit_rmdir_tree_bg(old_baseline);
    project_become_undo();
    str_free(old_baseline);

    //
    // throw away the development directory
    //
    if (change_was_a_branch(cp))
    {
	project_become(pp);
	commit_rmdir_tree_bg(dev_dir);
	project_become_undo();
    }
    else
    {
	devup = user_symbolic(pp, dev_name);
	user_become(devup);
	commit_rmdir_tree_bg(dev_dir);
	user_become_undo();
	user_free(devup);
    }
    str_free(dev_dir);

    tml.list = 0;
    tml.len = 0;
    tml.max = 0;
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
	//	    this_is_a_bug();
	//
#if MTIME_BLURB
	if (tml.len > 0)
	{
	    char		buf1[30];
	    char		buf2[30];

	    strcpy(buf1, ctime(&tml.list[0].old));
	    strcpy(buf2, ctime(&tml.list[tml.len - 1].old));
	    error_raw
	    (
		"original times range from %.24s to %.24s = %d seconds",
		buf1,
		buf2,
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
	    char		buf1[30];
	    char		buf2[30];

	    strcpy(buf1, ctime(&tml.list[0].becomes));
	    strcpy(buf2, ctime(&tml.list[tml.len - 1].becomes));
	    error_raw
	    (
		"adjusted times range from %.24s to %.24s = %d seconds",
		buf1,
		buf2,
		(int)(1 + tml.list[tml.len - 1].becomes - tml.list[0].becomes)
	    );
	}
#endif
	dir_walk(id, time_map_set, &tml);
	project_become_undo();

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
	time_final = now();
	if (tml.len > 0 && time_final < tml.list[tml.len - 1].becomes)
	{
	    long            nsec;
	    sub_context_ty  *scp;

	    nsec = tml.list[tml.len - 1].becomes - time_final;
	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", nsec);
	    error_intl(scp, i18n("throttling $number seconds"));
	    sub_context_delete(scp);
	    os_interrupt_register();
	    sleep(nsec);
	    if (os_interrupt_has_occurred())
	    {
		now_clear();
		time_final = now();
		if (time_final < tml.list[tml.len - 1].becomes)
		    goto impatient;
	    }
	}
	break;

    case pconf_build_time_adjust_adjust_only:
	//
	// warn the user if some files have been timed into the future
	//
	time_final = now();
	if (tml.len > 0 && time_final < tml.list[tml.len - 1].becomes)
	{
	    sub_context_ty	*scp;
	    long		nsec;

	    impatient:
	    scp = sub_context_new();
	    nsec = tml.list[tml.len - 1].becomes - time_final;
	    sub_var_set_long(scp, "Number", nsec);
	    sub_var_optional(scp, "Number");
	    error_intl(scp, i18n("warning: file times in future"));
	    sub_context_delete(scp);
	}
	break;
    }

    //
    // Write the change table row.
    // Write the user table row.
    // Release advisory locks.
    //
    change_cstate_write(cp);
    user_ustate_write(up);
    project_pstate_write(pp);
    change_verbose(cp, 0, i18n("discard old directories"));
    commit();
    lock_release();

    //
    // run the notify command
    //
    // Note that we set the AEGIS_INTEGRATION_DIRECTORY environment
    // variable further back in the code.
    //
    change_run_integrate_pass_notify_command(cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("integrate pass complete"));
    change_free(cp);
    user_free(pup);
    project_free(pp);
    user_free(up);

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
	{arglex_token_help, integrate_pass_help, },
	{arglex_token_list, integrate_pass_list, },
    };

    trace(("integrate_pass()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_pass_main);
    trace(("}\n"));
}
