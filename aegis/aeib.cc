//
//      aegis - project change supervisor
//      Copyright (C) 1991-2008 Peter Miller
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
#include <common/ac/string.h>
#include <common/ac/time.h>
#include <common/ac/unistd.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/error.h>
#include <common/gmatch.h>
#include <common/nstring.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/change/attributes.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/file.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/log.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/file.h>
#include <libaegis/project/history.h>
#include <libaegis/rss.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aeib.h>


static void
integrate_begin_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "usage: %s -Integrate_Begin [ <option>... ]\n", progname);
    fprintf
    (
        stderr,
        "       %s -Integrate_Begin -List [ <option>... ]\n",
        progname
    );
    fprintf(stderr, "       %s -Integrate_Begin -Help\n", progname);
    quit(1);
}


static void
integrate_begin_help(void)
{
    help("aeib", integrate_begin_usage);
}


static void
integrate_begin_list(void)
{
    trace(("integrate_begin_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(integrate_begin_usage);
    list_changes_in_state_mask(cid, 1 << cstate_state_awaiting_integration);
    trace(("}\n"));
}


static string_ty *
remove_comma_d_if_present(string_ty *s)
{
    char            *cp;

    if (s->str_length < 2)
        return str_copy(s);
    cp = s->str_text + s->str_length - 2;
    if (cp[0] == ',' && cp[1] == 'D')
        return str_n_from_c(s->str_text, s->str_length - 2);
    return str_copy(s);
}


static int
isa_suppressed_filename(change::pointer cp, string_ty *fn)
{
    pconf_ty        *pconf_data;
    pconf_integrate_begin_exceptions_list_ty *p;
    size_t          j;

    pconf_data = change_pconf_get(cp, 1);
    p = pconf_data->integrate_begin_exceptions;
    if (!p)
        return 0;
    for (j = 0; j < p->length; ++j)
    {
        if (gmatch(p->list[j]->str_text, fn->str_text))
            return 1;
    }
    return 0;
}


static void
chmod_common(string_ty *filename, const struct stat *st, int rdwr,
    change::pointer )
{
    int             mode;

    if (rdwr)
    {
        //
        // If it is not a source file, it could be owned
        // by some other user, and we have no control
        // over its owner or mode.  Report a warning if
        // we can't change the mode.
        //
        // Also, we leave it writable if it is already.
        // This is normal for generated files.
        //
        mode = (st->st_mode | 0644) & ~0022;
    }
    else
    {
        //
        // Source files, on the other hand, should always
        // be owned by us, and thus always chmod(2)able
        // by us.  Have a hissy-fit if they aren't.
        //
        // Also, source files should be read-only.
        //
        mode = (st->st_mode | 0444) & ~0222;
    }
    os_chmod(filename, mode);
}


static void
link_tree_callback_minimum(void *arg, dir_walk_message_ty message,
    string_ty *path, const struct stat *st)
{
    string_ty       *s1;
    string_ty       *s1short;
    string_ty       *s2;
    change::pointer cp;
    fstate_src_ty   *src;
    int             exists;
    int             remove_the_file;

    trace(("link_tree_callback_minimum(message = %d, path = %08lX, "
        "st = %08lX)\n{\n", message, (long)path, (long)st));
    os_interrupt_cope();
    cp = (change::pointer )arg;
    assert(cp);
    trace_string(path->str_text);
    s1 = os_below_dir(cp->pp->baseline_path_get(true), path);
    assert(s1);
    trace_string(s1->str_text);
    if (!s1->str_length)
        s2 = str_copy(change_integration_directory_get(cp, 1));
    else
        s2 = os_path_join(change_integration_directory_get(cp, 1), s1);
    trace_string(s2->str_text);
    switch (message)
    {
    case dir_walk_dir_before:
        if (!s1->str_length)
        {
            assert(!os_exists(s2));
            os_mkdir(s2, 02755);
        }
        break;

    case dir_walk_file:
        if (st->st_mode & 07000)
        {
            //
            // Don't link files with horrible modes.
            // They shouldn't be source, anyway.
            //
            project_become_undo(cp->pp);
            exists = !!project_file_find(cp->pp, s1, view_path_extreme);
            project_become(cp->pp);
            if (!exists)
                break;
        }

        //
        // Remove the ,D suffix, if present, and use the
        // shortened form to test for project and change
        // membership.  This ensures that diff files are also
        // copied across.  Note that deleted files *have*
        // difference files.
        //
        s1short = remove_comma_d_if_present(s1);

        //
        // Don't link it if it's not a source file, or a
        // relevant ,D file.
        //
        project_become_undo(cp->pp);
        src = project_file_find(cp->pp, s1short, view_path_simple);
        project_become(cp->pp);
        if
        (
            !src
        ||
            (src->deleted_by && str_equal(s1, s1short))
        )
        {
            str_free(s1short);
            break;
        }

        //
        // make sure the directory is there
        //
        os_mkdir_between(change_integration_directory_get(cp, 1), s1, 02755);

        //
        // Don't link a file (or the corresponding ,D file) if
        // the file is in the change.
        //
        if (change_file_find(cp, s1short, view_path_first))
        {
            str_free(s1short);
            break;
        }

        //
        // Don't link a suppressed file.
        // BUT keep primary source files and their diff files.
        //
        project_become_undo(cp->pp);
        remove_the_file =
            (
                !project_file_find(cp->pp, s1short, view_path_simple)
            &&
                isa_suppressed_filename(cp, s1short)
            );
        project_become(cp->pp);
        if (remove_the_file)
        {
            str_free(s1short);
            break;
        }
        str_free(s1short);

        //
        // link the file and make sure it is a suitable mode
        //
        trace(("ln %s %s\n", path->str_text, s2->str_text));
        os_link(path, s2);
        project_become_undo(cp->pp);
        exists = !!project_file_find(cp->pp, s1, view_path_simple);
        project_become(cp->pp);
        chmod_common(s2, st, !exists, cp);

        //
        // Update the modify time of the linked file.  On a
        // fully-functional Unix, this is unnecessary, because a
        // hard link alters the ctime, not the mtime, and this
        // is a no-op.  Solaris, on the other hand, is brain dead.
        //
        os_mtime_set(s2, st->st_mtime);
        break;

    case dir_walk_dir_after:
        break;

    case dir_walk_special:
    case dir_walk_symlink:
        //
        // ignore special files
        //
        // They could never be source files, so they must be
        // created by the build.  These ones must always be
        // created at build time, that's all.
        //
        break;
    }
    str_free(s2);
    str_free(s1);
    trace(("}\n"));
}


static void
link_tree_callback(void *arg, dir_walk_message_ty message, string_ty *path,
    const struct stat *st)
{
    string_ty       *s1;
    string_ty       *s1short;
    string_ty       *s2;
    change::pointer cp;
    fstate_src_ty   *src;
    string_ty       *contents;
    int             remove_the_file;

    trace(("link_tree_callback(message = %d, path = %08lX, st = %08lX)\n{\n",
        message, (long)path, (long)st));
    os_interrupt_cope();
    cp = (change::pointer )arg;
    assert(cp);
    trace_string(path->str_text);
    s1 = os_below_dir(cp->pp->baseline_path_get(true), path);
    assert(s1);
    trace_string(s1->str_text);
    if (!s1->str_length)
        s2 = str_copy(change_integration_directory_get(cp, 1));
    else
        s2 = os_path_join(change_integration_directory_get(cp, 1), s1);
    trace_string(s2->str_text);
    switch (message)
    {
    case dir_walk_dir_before:
        assert(!os_exists(s2));
        os_mkdir(s2, 02755);
        break;

    case dir_walk_file:
        project_become_undo(cp->pp);
        src = project_file_find(cp->pp, s1, view_path_extreme);
        project_become(cp->pp);
        if (st->st_mode & 07000)
        {
            //
            // Don't link files with horrible modes.
            // They shouldn't be source, anyway.
            //
            if (!src)
                break;
        }

        //
        // Don't link a file (or the corresponding ,D file) if
        // the file is in the change.
        //
        s1short = remove_comma_d_if_present(s1);
        if (change_file_find(cp, s1short, view_path_first))
        {
            str_free(s1short);
            break;
        }

        //
        // Don't link a suppressed file.
        // BUT keep primary source files and their diff files.
        //
        project_become_undo(cp->pp);
        remove_the_file =
            (
                !project_file_find(cp->pp, s1short, view_path_simple)
            &&
                isa_suppressed_filename(cp, s1short)
            );
        project_become(cp->pp);
        if (remove_the_file)
        {
            str_free(s1short);
            break;
        }
        str_free(s1short);

        //
        // link the file and make sure it is a suitable mode
        //
        os_link(path, s2);
        chmod_common(s2, st, !src, cp);

        //
        // Update the modify time of the linked file.  On a
        // fully-functional Unix, this is unnecessary, because a
        // hard link alters the ctime, not the mtime, and this
        // is a no-op.  Solaris, on the other hand, is brain dead.
        //
        os_mtime_set(s2, st->st_mtime);
        break;

    case dir_walk_dir_after:
        break;

    case dir_walk_symlink:
        contents = os_readlink(path);
        os_symlink(contents, s2);
        str_free(contents);
        break;

    case dir_walk_special:
        //
        // ignore special files
        //
        // They could never be source files, so they must be
        // created by the build.  These ones must always be
        // created at build time, that's all.
        //
        break;
    }
    str_free(s2);
    str_free(s1);
    trace(("}\n"));
}


static void
copy_tree_callback_minimum(void *arg, dir_walk_message_ty message,
    string_ty *path, const struct stat *st)
{
    string_ty       *s1;
    string_ty       *s1short;
    string_ty       *s2;
    change::pointer cp;
    fstate_src_ty   *src;
    int             uid;
    int             exists;
    int             remove_the_file;

    trace(("copy_tree_callback_minimum(message = %d, path = %08lX, "
        "st = %08lX)\n{\n", message, (long)path, (long)st));
    os_interrupt_cope();
    cp = (change::pointer )arg;
    assert(cp);
    trace_string(path->str_text);
    s1 = os_below_dir(cp->pp->baseline_path_get(true), path);
    assert(s1);
    trace_string(s1->str_text);
    if (!s1->str_length)
        s2 = str_copy(change_integration_directory_get(cp, 1));
    else
        s2 = os_path_join(change_integration_directory_get(cp, 1), s1);
    trace_string(s2->str_text);
    switch (message)
    {
    case dir_walk_dir_before:
        if (!s1->str_length)
        {
            assert(!os_exists(s2));
            os_mkdir(s2, 02755);
        }
        break;

    case dir_walk_file:
        //
        // Don't copy files which don't belong to us.
        // Don't copy files with horrible modes.
        // They shouldn't be source, anyway.
        //
        os_become_query(&uid, (int *)0, (int *)0);
        if
        (
            ((unsigned int)st->st_uid != (unsigned int)uid)
        ||
            (st->st_mode & 07000) != 0
        )
        {
            project_become_undo(cp->pp);
            exists = !!project_file_find(cp->pp, s1, view_path_extreme);
            project_become(cp->pp);
            if (!exists)
                break;
        }

        //
        // Remove the ,D suffix, if present, and use the
        // shortened form to test for project and change
        // membership.  This ensures that diff files are also
        // copied across.  Note that deleted files *have*
        // difference files.
        //
        s1short = remove_comma_d_if_present(s1);
        trace_string(s1short->str_text);

        //
        // Don't copy it if it's not a source file, or a
        // relevant ,D file.
        //
        project_become_undo(cp->pp);
        src = project_file_find(cp->pp, s1short, view_path_simple);
        remove_the_file =
            (
                !src
            ||
                (src->deleted_by && str_equal(s1, s1short))
            );
        project_become(cp->pp);
        if (remove_the_file)
        {
            str_free(s1short);
            break;
        }

        //
        // make sure the directory is there
        //
        os_mkdir_between(change_integration_directory_get(cp, 1), s1, 02755);

        //
        // Don't copy a file (or the corresponding ,D file) if
        // the file is in the change.
        //
        if (change_file_find(cp, s1short, view_path_first))
        {
            str_free(s1short);
            break;
        }

        //
        // Don't copy a suppressed file.
        // BUT keep primary source files and their diff files.
        //
        project_become_undo(cp->pp);
        remove_the_file =
            (
                !project_file_find(cp->pp, s1short, view_path_extreme)
            &&
                isa_suppressed_filename(cp, s1short)
            );
        project_become(cp->pp);
        if (remove_the_file)
        {
            str_free(s1short);
            break;
        }
        str_free(s1short);

        //
        // copy the file
        //
        trace(("cp %s %s\n", path->str_text, s2->str_text));
        copy_whole_file(path, s2, 1);
        project_become_undo(cp->pp);
        exists = !!project_file_find(cp->pp, s1, view_path_extreme);
        project_become(cp->pp);
        chmod_common(s2, st, !exists, cp);
        break;

    case dir_walk_dir_after:
        break;

    case dir_walk_special:
    case dir_walk_symlink:
        //
        // ignore special files
        //
        // They could never be source files, so they must be
        // created by the build.  These ones must always be
        // created at build time, that's all.
        //
        break;
    }
    str_free(s2);
    str_free(s1);
    trace(("}\n"));
}


static void
copy_tree_callback(void *arg, dir_walk_message_ty message, string_ty *path,
    const struct stat *st)
{
    string_ty       *s1;
    string_ty       *s1short;
    string_ty       *s2;
    change::pointer cp;
    int             uid;
    string_ty       *contents;
    int             exists;
    int             remove_the_file;

    trace(("copy_tree_callback(message = %d, path = %08lX, st = %08lX)\n{\n",
        message, (long)path, (long)st));
    os_interrupt_cope();
    cp = (change::pointer )arg;
    assert(cp);
    trace_string(path->str_text);
    s1 = os_below_dir(cp->pp->baseline_path_get(true), path);
    assert(s1);
    trace_string(s1->str_text);
    if (!s1->str_length)
        s2 = str_copy(change_integration_directory_get(cp, 1));
    else
        s2 = os_path_join(change_integration_directory_get(cp, 1), s1);
    trace_string(s2->str_text);
    switch (message)
    {
    case dir_walk_dir_before:
        assert(!os_exists(s2));
        os_mkdir(s2, 02755);
        break;

    case dir_walk_file:
        //
        // Don't copy files which don't belong to us.
        // Don't copy files with horrible modes.
        // They shouldn't be source, anyway.
        //
        os_become_query(&uid, (int *)0, (int *)0);
        if
        (
            (unsigned int)st->st_uid != (unsigned int)uid
        ||
            (st->st_mode & 07000) != 0
        )
        {
            project_become_undo(cp->pp);
            exists = !!project_file_find(cp->pp, s1, view_path_extreme);
            project_become(cp->pp);
            if (!exists)
                break;
        }

        //
        // Remove the ,D suffix, if present, and use the
        // shortened form to test for project and change
        // membership.  This ensures that diff files are also
        // copied across.  Note that deleted files *have*
        // difference files.
        //
        s1short = remove_comma_d_if_present(s1);
        trace_string(s1short->str_text);

        //
        // Don't copy a file (or the corresponding ,D file) if
        // the file is in the change.
        //
        if (change_file_find(cp, s1short, view_path_first))
        {
            str_free(s1short);
            break;
        }

        //
        // Don't copy a suppressed file.
        // BUT keep primary source files and their diff files.
        //
        project_become_undo(cp->pp);
        remove_the_file =
            (
                !project_file_find(cp->pp, s1short, view_path_extreme)
            &&
                isa_suppressed_filename(cp, s1short)
            );
        project_become(cp->pp);
        if (remove_the_file)
        {
            str_free(s1short);
            break;
        }
        str_free(s1short);

        //
        // copy the file
        //
        copy_whole_file(path, s2, 1);
        project_become_undo(cp->pp);
        exists = !!project_file_find(cp->pp, s1, view_path_extreme);
        project_become(cp->pp);
        chmod_common(s2, st, !exists, cp);
        break;

    case dir_walk_dir_after:
        break;

    case dir_walk_symlink:
        contents = os_readlink(path);
        os_symlink(contents, s2);
        str_free(contents);
        break;

    case dir_walk_special:
        //
        // ignore special files
        //
        // They could never be source files, so they must be
        // created by the build.  These ones must always be
        // created at build time, that's all.
        //
        break;
    }
    str_free(s2);
    str_free(s1);
    trace(("}\n"));
}


static void
integrate_begin_main(void)
{
    string_ty       *bl;
    string_ty       *dd;
    string_ty       *id;
    pconf_ty        *pconf_data;
    cstate_ty       *cstate_data;
    int             j;
    cstate_history_ty *history_data;
    bool            minimum;
    bool            maximum;
    string_ty       *project_name;
    project_ty      *pp;
    long            change_number;
    change::pointer cp;
    user_ty::pointer up;
    user_ty::pointer pup;
    int             errs;
    string_ty       *s;
    long            other;
    log_style_ty    log_style;
    string_ty       *base;
    int             base_max;
    string_ty       *num;
    int             mode;

    trace(("integrate_begin_main()\n{\n"));
    arglex();
    minimum = false;
    maximum = false;
    project_name = 0;
    change_number = 0;
    log_style = log_style_create_default;
    string_ty *reason = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(integrate_begin_usage);
            continue;

        case arglex_token_change:
            arglex();
            // fall through...

        case arglex_token_number:
            arglex_parse_change
            (
                &project_name,
                &change_number,
                integrate_begin_usage
            );
            continue;

        case arglex_token_minimum:
            if (minimum)
                duplicate_option(integrate_begin_usage);
            minimum = true;
            break;

        case arglex_token_maximum:
            if (maximum)
                duplicate_option(integrate_begin_usage);
            maximum = true;
            break;

        case arglex_token_project:
            arglex();
            // fall through...

        case arglex_token_string:
            arglex_parse_project(&project_name, integrate_begin_usage);
            continue;

        case arglex_token_nolog:
            if (log_style == log_style_none)
                duplicate_option(integrate_begin_usage);
            log_style = log_style_none;
            break;

        case arglex_token_wait:
        case arglex_token_wait_not:
            user_ty::lock_wait_argument(integrate_begin_usage);
            break;

	case arglex_token_reason:
	    if (reason)
		duplicate_option(integrate_begin_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_string(arglex_token_reason, integrate_begin_usage);
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
    trace_long(change_number);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // lock the project, the change and the user
    //
    pp->pstate_lock_prepare();
    change_cstate_lock_prepare(cp);
    up->ustate_lock_prepare();
    lock_take();
    cstate_data = cp->cstate_get();
    pconf_data = change_pconf_get(cp, 1);

    //
    // make sure they are allowed to
    //
    if (!project_integrator_query(pp, up->name()))
        project_fatal(pp, 0, i18n("not an integrator"));
    if (cstate_data->state != cstate_state_awaiting_integration)
        change_fatal(cp, 0, i18n("bad ib state"));
    if
    (
        !project_developer_may_integrate_get(pp)
    &&
        nstring(change_developer_name(cp)) == up->name()
    )
        change_fatal(cp, 0, i18n("developer may not integrate"));
    if
    (
        !project_reviewer_may_integrate_get(pp)
    &&
        nstring(change_reviewer_name(cp)) == up->name()
    )
        change_fatal(cp, 0, i18n("reviewer may not integrate"));

    //
    // make sure only one integration at a time
    // for each project
    //
    other = project_current_integration_get(pp);
    if (other)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_long(scp, "Number", magic_zero_decode(other));
        project_fatal(pp, scp, i18n("currently integrating $number"));
        // NOTREACHED
        sub_context_delete(scp);
    }
    trace_long(change_number);
    project_current_integration_set(pp, change_number);

    //
    // Look for additional integration hints from the change set.
    //
    if (!minimum && !maximum)
    {
	static string_ty *integrate_begin_hint;
	if (!integrate_begin_hint)
	    integrate_begin_hint = str_from_c("integrate-begin-hint");
	string_ty *vp = change_attributes_find(cp, integrate_begin_hint);
	if (vp)
	{
	    nstring value(vp);
	    if (value == "minimum")
		minimum = true;
	    if (value == "maximum")
		maximum = true;
	}
    }

    //
    // grab a delta number
    // and advance the project's delta counter
    //
    cstate_data->delta_number = project_next_delta_number(pp);
    cstate_data->delta_uuid = universal_unique_identifier();

    //
    // include the current year in the copyright_years field
    //
    change_copyright_years_now(cp);

    //
    // Create the integration directory.
    //
    base = str_format("delta%d", (int)getpid());
    num = str_format(".%3.3ld", cstate_data->delta_number);
    os_become_orig();
    base_max = os_pathconf_name_max(project_top_path_get(pp, 0));
    os_become_undo();
    base_max -= num->str_length;
    if (base_max < 5)
        base_max = 5;
    s =
        str_format
        (
            "%s/%.*s%s",
            project_top_path_get(pp, 0)->str_text,
            base_max,
            base->str_text,
            num->str_text
        );
    str_free(base);
    str_free(num);
    change_integration_directory_set(cp, s);
    str_free(s);

    //
    // There will be many files in the baseline in addition to the
    // sources files.
    //
    // If any files are being deleted, only copy the source files
    // from the baseline to the integration directory.  This way
    // the additional files relating to the removed sources file
    // are also removed (eg remove a .c file and you need to get
    // rid of the .o file).  It also shows when dependencies have
    // become out-of-date.
    //
    // It is possible to ask for this from the command line, too.
    //
    if (maximum && minimum)
    {
        mutually_exclusive_options
        (
            arglex_token_minimum,
            arglex_token_maximum,
            integrate_begin_usage
        );
    }
    if (!maximum && !minimum)
    {
        for (j = 0;; ++j)
        {
            fstate_src_ty   *src_data;

            src_data = change_file_nth(cp, j, view_path_first);
            if (!src_data)
                break;
            switch (src_data->action)
            {
            case file_action_create:
            case file_action_modify:
                break;

            case file_action_insulate:
                assert(0);
                break;

            case file_action_remove:
            case file_action_transparent:
                minimum = true;
                break;
            }
        }
    }

    //
    // Remember the minimum flag for the build command,
    // and also the aeipass command.
    //
    cstate_data->minimum_integration = minimum;

    //
    // before creating the integration directory,
    // make sure none of the change files have been tampered with.
    //
    // (a) the project owner must have access
    // (b) the changed file must exist and not have been modified
    // (c) the difference file must exist and not have been modified
    //
    dd = change_development_directory_get(cp, 1);
    id = change_integration_directory_get(cp, 1);
    bl = pp->baseline_path_get(true);
    errs = 0;
    for (j = 0;; ++j)
    {
        fstate_src_ty   *src_data;
        string_ty       *s1;
        string_ty       *s2;
        int             ok;

        src_data = change_file_nth(cp, j, view_path_first);
        if (!src_data)
            break;
        switch (src_data->usage)
        {
        case file_usage_source:
        case file_usage_config:
        case file_usage_test:
        case file_usage_manual_test:
            break;

        case file_usage_build:
            continue;
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
            if (change_was_a_branch(cp))
                continue;
            break;

        case file_action_remove:
            continue;
        }

        s1 = change_file_path(cp, src_data->file_name);
        project_become(pp);
        ok = change_fingerprint_same(src_data->file_fp, s1, 0);
        project_become_undo(pp);
        if (!ok)
        {
            sub_context_ty  *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", src_data->file_name);
            change_error(cp, scp, i18n("$filename altered"));
            sub_context_delete(scp);
            errs++;
        }
        assert(src_data->file_fp->youngest > 0);
        assert(src_data->file_fp->oldest > 0);

	if (change_diff_required(cp))
	{
	    //
            // Why doesn't this code follow the same logic about
            // diff_file_required as used by aede, aerb and aerpass?
	    //
	    assert(src_data->diff_file_fp);
	    s2 = str_format("%s,D", s1->str_text);
	    project_become(pp);
	    ok = change_fingerprint_same(src_data->diff_file_fp, s2, 0);
	    project_become_undo(pp);
	    if (!ok)
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
		change_error(cp, scp, i18n("$filename altered"));
		sub_context_delete(scp);
		errs++;
	    }
	    str_free(s2);
	    assert(src_data->diff_file_fp->youngest > 0);
	    assert(src_data->diff_file_fp->oldest > 0);
	}
        str_free(s1);
    }
    if (errs)
        quit(1);

    //
    // add to the change history
    //      (The time stamp is used later for the file mod times.)
    //
    os_throttle();
    history_data = change_history_new(cp, up);
    history_data->what = cstate_history_what_integrate_begin;
    history_data->why = reason;

    //
    // Make sure the integration directory is not already there
    // (a user make have killed a previous aeib).  Register a rmdir
    // (to be done in the background) if an undo is needed.
    //
    project_become(pp);
    if (os_exists(id))
        os_rmdir_tree(id);
    undo_rmdir_bg(id);

    //
    // create the integration directory
    // copy everything from baseline to integration directory
    // except things from the change
    // and change owner to the project's owner.
    //
    if (pconf_data->link_integration_directory)
    {
        change_verbose(cp, 0, i18n("link baseline to integration directory"));
        dir_walk
        (
            bl,
            (minimum ? link_tree_callback_minimum : link_tree_callback),
            cp
        );
    }
    else
    {
        change_verbose(cp, 0, i18n("copy baseline to integration directory"));
        dir_walk
        (
            bl,
            (minimum ? copy_tree_callback_minimum : copy_tree_callback),
            cp
        );
    }

    //
    // apply the changes to the integration directory
    //
    change_verbose(cp, 0, i18n("apply change to integration directory"));
    for (j = 0;; ++j)
    {
        fstate_src_ty   *src_data;
        string_ty       *s1;
        string_ty       *s2;

        src_data = change_file_nth(cp, j, view_path_first);
        if (!src_data)
            break;
        s1 = os_path_join(dd, src_data->file_name);
        s2 = os_path_join(id, src_data->file_name);
        if (os_exists(s2))
        {
            //
            // this is defensive,
            // and should never need to be executed
            //
            os_unlink(s2);
        }
        switch (src_data->action)
        {
        case file_action_insulate:
            //
            // This should never happen: aede will fail if
            // there are any insulation files.
            //
            assert(0);
            break;

        case file_action_remove:
        case file_action_transparent:
            break;

        case file_action_modify:
        case file_action_create:
            switch (src_data->usage)
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
                //
                // New files do not exist in the baseline,
                // and old files may not be copied under -MINimum,
                // so we may need to create directories.
                //
                os_mkdir_between(id, src_data->file_name, 02755);
                copy_whole_file(s1, s2, 0);

                //
                // Set the mode of the file.
                //
                mode = 0444;
                if (os_executable(s1))
                    mode |= 0111;
                os_chmod(s2, mode & ~change_umask(cp));

                //
                // Make all of the change's files have the same
                // mod time, so that when aeipass flattens the
                // mod times, all of them fall into a single
                // second, minimizing the chance that mod times
                // will extend into the future after aeipass.
                //
                // This also helps cooperating DMTs flatten the
                // targets' mod times into a smaller range,
                // which also helps aeipass.
                //
                os_mtime_set(s2, history_data->when);
                break;
            }
            break;
        }
        str_free(s1);
        str_free(s2);

        //
        // clear the file's test times
        //
        if (src_data->architecture_times)
        {
            fstate_src_architecture_times_list_type.free
            (
                src_data->architecture_times
            );
            src_data->architecture_times = 0;
        }

        //
        // clear the file's integrate difference time
        //
        if (src_data->idiff_file_fp)
        {
            fingerprint_type.free(src_data->idiff_file_fp);
            src_data->idiff_file_fp = 0;
        }
    }
    project_become_undo(pp);

    //
    // add the change to the user's list
    //
    trace_long(change_number);
    up->own_add(pp, change_number);
    cstate_data->state = cstate_state_being_integrated;

    //
    // clear the change build times,
    // and test times
    //
    change_build_times_clear(cp);

    //
    // write the data out
    // and release the locks
    //
    pp->pstate_write();
    change_cstate_write(cp);
    up->ustate_write();
    commit();
    lock_release();

    //
    // run the integrate begin command
    //
    pup = project_user(pp);
    log_open(change_logfile_get(cp), pup, log_style);
    change_run_integrate_begin_command(cp);

    //
    // Update the RSS feed file if necessary.
    //
    rss_add_item_by_change(pp, cp);

    //
    // verbose success message
    //
    change_verbose(cp, 0, i18n("integrate begin complete"));
    change_free(cp);
    project_free(pp);
    trace(("}\n"));
}


void
integrate_begin()
{
    static arglex_dispatch_ty dispatch[] =
    {
        { arglex_token_help, integrate_begin_help, 0 },
        { arglex_token_list, integrate_begin_list, 0 },
    };

    trace(("integrate_begin()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), integrate_begin_main);
    trace(("}\n"));
}
