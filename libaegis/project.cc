//
//	aegis - project change supervisor
//	Copyright (C) 1991-2008 Peter Miller
//      Copyright (C) 2007 Walter Franzini
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
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

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/string.h>
#include <common/ac/stdlib.h>

#include <common/error.h>
#include <common/fstrcmp.h>
#include <common/itab.h>
#include <common/mem.h>
#include <common/skip_unlucky.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/commit.h>
#include <libaegis/gonzo.h>
#include <libaegis/lock.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/pstate.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


void
project_ty::convert_to_new_format()
{
    trace(("project_ty::convert_to_new_format(this = %08lX)\n{\n", (long)this));
    // Don't worry about is not being NULL, it won't contain much,
    // and it will only be a memory leak when the project is first
    // converted, and will never happen again.
    pcp = change_alloc(this, TRUNK_CHANGE_NUMBER);
    change_bind_new(pcp);
    change_branch_new(pcp);

    //
    // The new change is in the 'being developed'
    // state, and will be forever.
    //
    assert(up);
    cstate_history_ty *h = change_history_new(pcp, up);
    h->what = cstate_history_what_new_change;
    h = change_history_new(pcp, up);
    h->what = cstate_history_what_develop_begin;
    pcp->cstate_data->state = cstate_state_being_developed;

    //
    // set the development directory
    //
    change_development_directory_set(pcp, home_path_get());

    //
    // Copy the information from the old format of project state
    // into the newly created change.  Remember to clear the old
    // stuff out, so that it isn't written back to the file.
    //
    pstate_get();

    change_branch_umask_set(pcp, pstate_data->umask);
    pstate_data->umask = 0;

    if (pstate_data->owner_name)
    {
	// this field is ignored
	str_free(pstate_data->owner_name);
	pstate_data->owner_name = 0;
    }

    if (pstate_data->group_name)
    {
	// this field is ignored
	str_free(pstate_data->group_name);
	pstate_data->group_name = 0;
    }

    if (pstate_data->description)
    {
	project_description_set(this, pstate_data->description);
	str_free(pstate_data->description);
	pstate_data->description = 0;
    }

    change_branch_developer_may_review_set
    (
	pcp,
	pstate_data->developer_may_review
    );
    pstate_data->developer_may_review = false;

    change_branch_developer_may_integrate_set
    (
	pcp,
	pstate_data->developer_may_integrate
    );
    pstate_data->developer_may_integrate = false;

    change_branch_reviewer_may_integrate_set
    (
	pcp,
	pstate_data->reviewer_may_integrate
    );
    pstate_data->reviewer_may_integrate = false;

    change_branch_developers_may_create_changes_set
    (
	pcp,
	pstate_data->developers_may_create_changes
    );
    pstate_data->developers_may_create_changes = false;

    //
    // Old-style projects alawys had regression tests exempt by default.
    //
    change_branch_default_test_regression_exemption_set(pcp, true);

    change_branch_forced_develop_begin_notify_command_set
    (
	pcp,
	pstate_data->forced_develop_begin_notify_command
    );
    pstate_data->forced_develop_begin_notify_command = 0;

    change_branch_develop_end_notify_command_set
    (
	pcp,
	pstate_data->develop_end_notify_command
    );
    pstate_data->develop_end_notify_command = 0;

    change_branch_develop_end_undo_notify_command_set
    (
	pcp,
	pstate_data->develop_end_undo_notify_command
    );
    pstate_data->develop_end_undo_notify_command = 0;

    change_branch_review_pass_notify_command_set
    (
	pcp,
	pstate_data->review_pass_notify_command
    );
    pstate_data->review_pass_notify_command = 0;

    change_branch_review_pass_undo_notify_command_set
    (
	pcp,
	pstate_data->review_pass_undo_notify_command
    );
    pstate_data->review_pass_undo_notify_command = 0;

    change_branch_review_fail_notify_command_set
    (
	pcp,
	pstate_data->review_fail_notify_command
    );
    pstate_data->review_fail_notify_command = 0;

    change_branch_integrate_pass_notify_command_set
    (
	pcp,
	pstate_data->integrate_pass_notify_command
    );
    pstate_data->integrate_pass_notify_command = 0;

    change_branch_integrate_fail_notify_command_set
    (
	pcp,
	pstate_data->integrate_fail_notify_command
    );
    pstate_data->integrate_fail_notify_command = 0;

    change_branch_default_development_directory_set
    (
	pcp,
	pstate_data->default_development_directory
    );
    pstate_data->default_development_directory = 0;

    change_branch_default_test_exemption_set
    (
	pcp,
	pstate_data->default_test_exemption
    );
    pstate_data->default_test_exemption = false;

    if (!pstate_data->copyright_years)
    {
	pstate_data->copyright_years =
	    (pstate_copyright_years_list_ty *)
            pstate_copyright_years_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->copyright_years->length; ++j)
    {
	change_copyright_year_append
	(
	    pcp,
	    pstate_data->copyright_years->list[j]
	);
    }
    pstate_copyright_years_list_type.free(pstate_data->copyright_years);
    pstate_data->copyright_years = 0;

    // no explict next change number in new format
    pstate_data->next_change_number = 0;

    // no explict next delta number in new format
    pstate_data->next_delta_number = 0;

    if (!pstate_data->src)
    {
	pstate_data->src = (pstate_src_list_ty *)pstate_src_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->src->length; ++j)
    {
	pstate_src_ty *sp1 = pstate_data->src->list[j];
	if (!sp1->file_name)
	{
	    sub_context_ty  *scp;

	    yuck:
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", pstate_path);
	    sub_var_set_charstar(scp, "FieLD_Name", "src");
	    project_fatal
	    (
		this,
		scp,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	fstate_src_ty *sp2 = pcp->file_new(sp1->file_name);
	if (!(sp1->mask & pstate_src_usage_mask))
	    goto yuck;
	sp2->usage = sp1->usage;
	sp2->mask |= fstate_src_usage_mask;
	sp2->action = file_action_create;
	sp2->mask |= fstate_src_action_mask;
	if (sp1->edit_number)
	{
	    sp2->edit = (history_version_ty *)history_version_type.alloc();
	    sp2->edit->revision = str_copy(sp1->edit_number);
	    sp2->edit_origin =
                (history_version_ty *)history_version_type.alloc();
	    sp2->edit_origin->revision = str_copy(sp1->edit_number);
	}
	sp2->locked_by = sp1->locked_by;
	sp2->about_to_be_created_by = sp1->about_to_be_created_by;
	sp2->deleted_by = sp1->deleted_by;

	if (sp2->deleted_by)
	    sp2->action = file_action_remove;

	//
	// This code must agree with the corresponding code in
	// libaegis/change/file/fstate.c
	//
	switch (sp2->action)
	{
	case file_action_remove:
	case file_action_transparent:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    if (sp2->about_to_be_created_by || sp2->about_to_be_copied_by)
		sp2->action = file_action_transparent;
	    break;
	}
    }
    pstate_src_list_type.free(pstate_data->src);
    pstate_data->src = 0;

    if (!pstate_data->history)
    {
	pstate_data->history =
            (pstate_history_list_ty *)pstate_history_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->history->length; ++j)
    {
	pstate_history_ty *hp;

	hp = pstate_data->history->list[j];
	change_branch_history_new(pcp, hp->delta_number, hp->change_number);
    }
    pstate_history_list_type.free(pstate_data->history);
    pstate_data->history = 0;

    if (!pstate_data->change)
    {
	pstate_data->change =
            (pstate_change_list_ty *)pstate_change_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->change->length; ++j)
    {
	change_branch_change_add(pcp, pstate_data->change->list[j], 0);
    }
    pstate_change_list_type.free(pstate_data->change);
    pstate_data->change = 0;

    //
    // copy the staff across
    //
    pstate_get();
    if (!pstate_data->administrator)
    {
	pstate_data->administrator =
	    (pstate_administrator_list_ty *)
            pstate_administrator_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->administrator->length; ++j)
    {
	change_branch_administrator_add
	(
	    pcp,
	    pstate_data->administrator->list[j]
	);
    }
    pstate_administrator_list_type.free(pstate_data->administrator);
    pstate_data->administrator = 0;

    if (!pstate_data->developer)
    {
	pstate_data->developer =
	    (pstate_developer_list_ty *)pstate_developer_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->developer->length; ++j)
    {
	change_branch_developer_add(pcp, pstate_data->developer->list[j]);
    }
    pstate_developer_list_type.free(pstate_data->developer);
    pstate_data->developer = 0;

    if (!pstate_data->reviewer)
    {
	pstate_data->reviewer =
	    (pstate_reviewer_list_ty *)pstate_reviewer_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->reviewer->length; ++j)
    {
	change_branch_reviewer_add(pcp, pstate_data->reviewer->list[j]);
    }
    pstate_reviewer_list_type.free(pstate_data->reviewer);
    pstate_data->reviewer = 0;

    if (!pstate_data->integrator)
    {
	pstate_data->integrator =
	    (pstate_integrator_list_ty *)pstate_integrator_list_type.alloc();
    }
    for (size_t j = 0; j < pstate_data->integrator->length; ++j)
    {
	change_branch_integrator_add(pcp, pstate_data->integrator->list[j]);
    }
    pstate_integrator_list_type.free(pstate_data->integrator);
    pstate_data->integrator = 0;

    if (pstate_data->currently_integrating_change)
    {
	change_current_integration_set
	(
	    pcp,
	    pstate_data->currently_integrating_change
	);
	pstate_data->currently_integrating_change = 0;
    }

    //
    // These should actually be acted on to create a change tree,
    // but that will have to wait for a future aegis change, when
    // branching is working properly.
    //
    // pstate_data->version_major = 0;
    // pstate_data->version_minor = 0;
    //

    if (pstate_data->version_previous)
    {
	pcp->cstate_data->version_previous = pstate_data->version_previous;
	pstate_data->version_previous = 0;
    }

    //
    // By default we reuse change numbers.
    //
    change_branch_reuse_change_numbers_set(pcp, 1);

    //
    // Phew!  Who would ever have guessed there was so much to do
    // when it came to converting a project to use branching?
    //
    trace(("}\n"));
}


project_ty::~project_ty()
{
    trace(("project_ty::~project_ty(this = %08lX)\n{\n", (long)this));
    if (pcp)
	change_free(pcp);
    assert(name);
    str_free(name);
    if (home_path)
	str_free(home_path);
    if (baseline_path_unresolved)
	str_free(baseline_path_unresolved);
    if (baseline_path)
	str_free(baseline_path);
    if (change2time_stp)
        itab_free(change2time_stp);
    if (history_path)
	str_free(history_path);
    if (info_path)
	str_free(info_path);
    if (pstate_path)
	str_free(pstate_path);
    if (changes_path)
	str_free(changes_path);
    if (pstate_data)
	pstate_type.free(pstate_data);
    if (parent)
	project_free(parent);
    file_list_invalidate();
    trace(("}\n"));
}


project_ty::project_ty(string_ty *s) :
    reference_count(1),
    name(str_copy(s)),
    home_path(0),
    baseline_path_unresolved(0),
    baseline_path(0),
    history_path(0),
    info_path(0),
    pstate_path(0),
    changes_path(0),
    pstate_data(0),
    is_a_new_file(false),
    lock_magic(0),
    pcp(0),
    uid(-1),
    gid(-1),
    parent(0),
    parent_bn(0),
    off_limits(false)
{
    change2time_stp = itab_alloc();
    for (size_t j = 0; j < SIZEOF(file_list); ++j)
	file_list[j] = 0;
    for (size_t k = 0; k < SIZEOF(file_by_uuid); ++k)
	file_by_uuid[k] = 0;
}


project_ty *
project_alloc(string_ty *s)
{
    trace(("project_alloc(s = \"%s\")\n{\n", s->str_text));
    project_ty *pp = new project_ty(s);
    trace(("return %08lX;\n", (long)pp));
    trace(("}\n"));
    return pp;
}


project_ty *
project_copy(project_ty *pp)
{
    trace(("project_copy(pp = %08lX)\n{\n", (long)pp));
    assert(pp->reference_count >= 1);
    pp->reference_count++;
    trace(("return %08lX;\n", (long)pp));
    trace(("}\n"));
    return pp;
}


void
project_free(project_ty *pp)
{
    trace(("project_free(pp = %08lX)\n{\n", (long)pp));
    assert(pp->reference_count >= 1);
    pp->reference_count--;

    //
    // The root project's pcp references the root project (project_ty
    // data structure). Thus even as all other references go away, the
    // reference count of the root project remains at 1 and its memory
    // is not released. In the special case where reference_count is
    // one in project_free() we should test whether we're the root
    // project and the only thing holding on to us is the pcp. In that
    // case we should untie the loop and do a free on the pcp (which
    // will cascade to free us).
    //
    if (pp->reference_count == 1)
    {
        change::pointer tmp = pp->change_get_raw();

        if (tmp && tmp->pp == pp && pp->is_a_trunk())
            pp->change_reset();
    }
    else if (pp->reference_count <= 0)
    {
	delete pp;
    }
    trace(("}\n"));
}


void
project_pstate_lock_prepare_top(project_ty *pp)
{
    trace(("project_pstate_lock_prepare_top(pp = %08lX)\n{\n", (long)pp));
    pp->trunk_get()->pstate_lock_prepare();
    trace(("}\n"));
}


static void
waiting_for_baseline_read_lock(void *p)
{
    project_ty	    *pp;

    pp = (project_ty *)p;
    if (user_ty::create()->lock_wait())
	project_error(pp, 0, i18n("waiting for baseline read lock"));
    else
	project_fatal(pp, 0, i18n("baseline read lock not available"));
}


void
project_baseline_read_lock_prepare(project_ty *pp)
{
    //
    // A branch's baseline is "unioned" with its parent's
    // baseline, so we need to lock them as well - all the
    // way up the tree.
    //
    trace(("project_baseline_read_lock_prepare(pp = %08lX)\n{\n", (long)pp));
    assert(pp);
    for (;;)
    {
	lock_prepare_baseline_read
	(
	    pp->name_get(),
	    waiting_for_baseline_read_lock,
	    pp
	);
	if (pp->is_a_trunk())
	    break;
	pp = pp->parent_get();
    }
    trace(("}\n"));
}


static void
waiting_for_baseline_write_lock(void *p)
{
    project_ty	    *pp;

    pp = (project_ty *)p;
    if (user_ty::create()->lock_wait())
	project_error(pp, 0, i18n("waiting for baseline write lock"));
    else
	project_fatal(pp, 0, i18n("baseline write lock not available"));
}


void
project_baseline_write_lock_prepare(project_ty *pp)
{
    trace(("project_baseline_write_lock_prepare(pp = %08lX)\n{\n", (long)pp));
    lock_prepare_baseline_write
    (
	pp->name_get(),
	waiting_for_baseline_write_lock,
	pp
    );
    trace(("}\n"));
}


static void
waiting_for_history_lock(void *p)
{
    project_ty	    *pp;

    pp = (project_ty *)p;
    if (user_ty::create()->lock_wait())
	project_error(pp, 0, i18n("waiting for history lock"));
    else
	project_fatal(pp, 0, i18n("history lock not available"));
}


void
project_history_lock_prepare(project_ty *pp)
{
    //
    // The history tool may have no locking of its own, and it will
    // be ignored if it does.  Therefore, take the history lock in
    // the deepest project.  This prevents aeip on different
    // branches from trashing each other's history files.
    //
    trace(("project_history_lock_prepare(pp = %08lX)\n{\n", (long)pp));
    pp = pp->trunk_get();
    lock_prepare_history(pp->name_get(), waiting_for_history_lock, pp);
    trace(("}\n"));
}


int
break_up_version_string(const char *sp, long *buf, int buflen_max,
    int *buflen_p, int leading_punct)
{
    int		    buflen;
    long	    n;

    if (*sp == 0)
	return -1;
    buflen = *buflen_p;
    for (;;)
    {
	//
	// one leading punctuation character
	// but we don't care what it is (- and . are common)
	// {C locale}
	//
	if (leading_punct)
	{
	    if (!ispunct((unsigned char)*sp))
		return -1;
	    ++sp;
	}
	else
	    leading_punct = 1;

	//
	// the next one must be a digit
	//
	if (!isdigit((unsigned char)*sp))
	    return -1;

	//
	// The version string must not be too long.  (Even
	// oracle, who use the most unbelievable 6 part version
	// strings, will cope if you use a big enough buffer.)
	//
	if (buflen >= buflen_max)
	    return -1;

	//
	// collect the number
	//
	n = 0;
	for (;;)
	{
	    n = n * 10 + *sp++ - '0';
	    if (!isdigit((unsigned char)*sp))
		break;
	}
	buf[buflen++] = magic_zero_encode(n);

	//
	// stop at end of string
	// (note: trailing punctuation is illegal)
	//
	if (!*sp)
	    break;
    }
    *buflen_p = buflen;
    return 0;
}


void
extract_version_from_project_name(string_ty **name_p, long *buf, int buflen_max,
    int *buflen_p)
{
    string_ty	    *name;
    char	    *sp;
    int		    err;

    //
    // if the project name does not end in a digit,
    // it can't end in a version string
    //
    name = *name_p;
    if
    (
	name->str_length < 3
    ||
	// C locale
	!isdigit((unsigned char)name->str_text[name->str_length - 1])
    )
	return;
    sp = name->str_text;

    //
    // move down the name, looking for a trailing version number
    //
    for (; *sp; ++sp)
    {
	//
	// skip over leading non-punctuation
	// {C locale}
	//
	if (!ispunct((unsigned char)*sp))
	    continue;

	//
	// attempt to break up the rest of the string
	//
	err = break_up_version_string(sp, buf, buflen_max, buflen_p, 1);

	//
	// if there was an error, try again further down
	//
	if (err)
	    continue;

	//
	// the version number is now in the buffer,
	// so shorten the name to match
	//
	*name_p = str_n_from_c(name->str_text, sp - name->str_text);
	str_free(name);
	return;
    }
}


project_ty *
project_ty::find_branch(const char *version_string)
{
    long	    version[20];
    int		    version_length;
    int		    j;

    if (parent)
    {
	//
	// As a special case, the "grandparent" of a change is
	// repesented by a branch name of ".." as this is expected to be
	// the commonest variety of cross branch merge.
	//
	if (!strcmp(version_string, ".."))
	    return project_copy(parent);

	//
	// the version is relative to the trunk of the project
	//
	return trunk_get()->find_branch(version_string);
    }

    //
    // break the version string
    //
    version_length = 0;
    if
    (
	*version_string
    &&
	break_up_version_string
	(
	    version_string,
	    version,
	    (int)sizeof(version),
	    &version_length,
	    0
	)
    )
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Number", version_string);
	fatal_intl(scp, i18n("bad version $number"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (version_length == 0)
	return project_copy(this);

    //
    // follow the branches down
    //
    project_ty *pp = this;
    for (j = 0; j < version_length; ++j)
    {
	long		cn;
	change::pointer cp;

	cn = version[j];
	cp = change_alloc(pp, cn);
	change_bind_existing(cp);
	if (!change_was_a_branch(cp))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Number", version_string);
	    change_fatal(cp, scp, i18n("version $number not a branch"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	pp = pp->bind_branch(cp);
    }

    //
    // return the derived project
    //
    return pp;
}


void
project_ty::pstate_write()
{
    string_ty	    *filename;
    string_ty	    *filename_new;
    string_ty	    *filename_old;
    static int	    count;
    int		    compress;

    trace(("project_ty::pstate_write(thid = %08lX)\n{\n", (long)this));

    //
    // write out the associated change, if it was read in
    //
    if (pcp)
	change_cstate_write(pcp);

    //
    // write it out
    //
    compress = project_compress_database_get(this);
    if (pstate_data)
    {
	filename = pstate_path_get();
	filename_new = str_format("%s,%d", filename->str_text, ++count);
	filename_old = str_format("%s,%d", filename->str_text, ++count);
        user_ty::become scoped(get_user());

	// enums with 0 value not to be printed
	type_enum_option_set();

	if (is_a_new_file)
	{
	    undo_unlink_errok(filename_new);
	    pstate_write_file(filename_new, pstate_data, compress);
	    commit_rename(filename_new, filename);
	}
	else
	{
	    undo_unlink_errok(filename_new);
	    pstate_write_file(filename_new, pstate_data, compress);
	    commit_rename(filename, filename_old);
	    commit_rename(filename_new, filename);
	    commit_unlink_errok(filename_old);
	}

	//
	// Change so the project owns it.
	// (Only needed for new files, but be paranoid.)
	//
	os_chmod(filename_new, 0644 & ~project_umask_get(this));
	str_free(filename_new);
	str_free(filename_old);
    }
    trace(("}\n"));
}


void
project_pstate_write_top(project_ty *pp)
{
    trace(("project_pstate_write_top(pp = %08lX)\n{\n", (long)pp));
    pp->trunk_get()->pstate_write();
    trace(("}\n"));
}


string_ty *
project_Home_path_get(project_ty *pp)
{
    return pp->trunk_get()->home_path_get();
}


string_ty *
project_top_path_get(project_ty *pp, int resolve)
{
    change::pointer cp = pp->change_get();
    return change_top_path_get(cp, resolve);
}


nstring
project_rss_path_get(project_ty *pp, bool resolve)
{
    string_ty *project_top_path = project_top_path_get(pp, resolve);
    return os_path_cat(nstring(project_top_path), "rss");
}


void
project_error(project_ty *pp, sub_context_ty *scp, const char *s)
{
    string_ty	    *msg;
    int		    need_to_delete;

    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    //
    // form the message
    //
    subst_intl_project(scp, pp);
    msg = subst_intl(scp, s);

    //
    // pass the message to the error function
    //
    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", msg);
    str_free(msg);
    subst_intl_project(scp, pp);
    error_intl(scp, i18n("project \"$project\": $message"));

    if (need_to_delete)
	sub_context_delete(scp);
}


void
project_fatal(project_ty *pp, sub_context_ty *scp, const char *s)
{
    string_ty	    *msg;
    int		    need_to_delete;

    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    //
    // form the message
    //
    subst_intl_project(scp, pp);
    msg = subst_intl(scp, s);

    //
    // pass the message to the error function
    //
    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", msg);
    str_free(msg);
    subst_intl_project(scp, pp);
    fatal_intl(scp, i18n("project \"$project\": $message"));
    // NOTREACHED
    if (need_to_delete)
	sub_context_delete(scp);
}


void
project_verbose(project_ty *pp, sub_context_ty *scp, const char *s)
{
    string_ty	    *msg;
    int		    need_to_delete;

    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    //
    // form the message
    //
    subst_intl_project(scp, pp);
    msg = subst_intl(scp, s);

    //
    // pass the message to the error function
    //
    // re-use the substitution context
    sub_var_set_string(scp, "MeSsaGe", msg);
    str_free(msg);
    subst_intl_project(scp, pp);
    verbose_intl(scp, i18n("project \"$project\": $message"));

    if (need_to_delete)
	sub_context_delete(scp);
}


void
project_change_append(project_ty *pp, long cn, int is_a_branch)
{
    change::pointer cp = pp->change_get();
    change_branch_change_add(cp, cn, is_a_branch);
}


void
project_change_delete(project_ty *pp, long cn)
{
    change::pointer cp = pp->change_get();
    change_branch_change_remove(cp, cn);
}


int
project_change_number_in_use(project_ty *pp, long cn)
{
    change::pointer cp = pp->change_get();
    return change_branch_change_number_in_use(cp, cn);
}


string_ty *
project_version_short_get(project_ty *pp)
{
    string_ty	    *s;

    trace(("project_version_short_get(pp = %08lX)\n{\n", (long)pp));
    if (!pp->is_a_trunk())
    {
	assert(pp->parent_branch_number_get() > 0
               ||
               pp->parent_branch_number_get() == MAGIC_ZERO);
	s = project_version_short_get(pp->parent_get());
	if (s->str_length)
	{
	    string_ty	    *s2;

	    // ...punctuation?
	    s2 =
		str_format
		(
		    "%s.%ld",
		    s->str_text,
		    magic_zero_decode(pp->parent_branch_number_get())
		);
	    str_free(s);
	    s = s2;
	}
	else
	{
	    str_free(s);
	    s =
		str_format
		(
		    "%ld",
		    magic_zero_decode(pp->parent_branch_number_get())
		);
	}
    }
    else
    {
	pp->change_get(); // make sure is in memory
	pstate_ty *pstate_data = pp->pstate_get();
	if (pstate_data->version_major || pstate_data->version_minor)
	{
	    //
	    // old style project, not yet branching
	    //
	    s =
		str_format
		(
		    "%ld.%ld",
		    pstate_data->version_major,
		    pstate_data->version_minor
		);
	}
	else
	    s = str_from_c("");
    }
    trace(("return \"%s\";\n", s->str_text));
    trace(("}\n"));
    return s;
}


string_ty *
project_version_get(project_ty *pp)
{
    long	    dn;
    string_ty	    *result;
    string_ty	    *tmp;

    trace(("project_version_get(pp = %08lX)\n{\n", (long)pp));
    change::pointer cp = pp->change_get();
    dn = change_history_delta_latest(cp);
    tmp = project_version_short_get(pp);
    result = str_format("%s.D%3.3ld", tmp->str_text, dn);
    str_free(tmp);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


user_ty::pointer
project_user(project_ty *pp)
{
    return pp->get_user();
}


user_ty::pointer
project_ty::get_user()
    const
{
    trace(("project_ty::get_user(this = %08lX)\n{\n", (long)this));
    if (!up)
    {
        fatal_raw
        (
            "%s: %d: project_ty::up not set, you should have called "
            "project_bind_existing or project_ty::bind_new() before now "
            "(bug)",
            __FILE__,
            __LINE__
        );
    }
    trace(("return %08lX;\n", (long)up.get()));
    trace(("}\n"));
    return up;
}


void
project_become(project_ty *pp)
{
    trace(("project_become(pp = %08lX)\n{\n", (long)pp));
    pp->get_user()->become_begin();
    trace(("}\n"));
}


void
project_become_undo(project_ty *pp)
{
    trace(("project_become_undo(cp)\n{\n"));
    pp->get_user()->become_end();
    trace(("}\n"));
}


int
project_is_readable(project_ty *pp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    pp = pp->trunk_get();
    string_ty *s = pp->pstate_path_get();
    os_become_orig();
    int err = os_readable(s);
    os_become_undo();
    trace(("return %d\n", err));
    return err;
}


long
project_next_test_number_get(project_ty *pp)
{
    pstate_ty	    *pstate_data;
    long	    result;
    int		    skip;

    trace(("project_next_test_number_get(pp = %08lX)\n{\n", (long)pp));
    skip = project_skip_unlucky_get(pp);
    pp = pp->trunk_get();
    pstate_data = pp->pstate_get();
    result = pstate_data->next_test_number;
    if (skip)
	result = skip_unlucky(result);
    pstate_data->next_test_number = result + 1;
    trace(("return %ld;\n", result));
    trace(("}\n"));
    return result;
}


long
project_minimum_change_number_get(project_ty *pp)
{
    change::pointer cp = pp->change_get();
    return change_branch_minimum_change_number_get(cp);
}


void
project_minimum_change_number_set(project_ty *pp, long n)
{
    change::pointer cp = pp->change_get();
    change_branch_minimum_change_number_set(cp, n);
}


bool
project_reuse_change_numbers_get(project_ty *pp)
{
    change::pointer cp = pp->change_get();
    return change_branch_reuse_change_numbers_get(cp);
}


void
project_reuse_change_numbers_set(project_ty *pp, bool n)
{
    change::pointer cp = pp->change_get();
    change_branch_reuse_change_numbers_set(cp, n);
}


long
project_minimum_branch_number_get(project_ty *pp)
{
    change::pointer cp = pp->change_get();
    return change_branch_minimum_branch_number_get(cp);
}


void
project_minimum_branch_number_set(project_ty *pp, long n)
{
    change::pointer cp = pp->change_get();
    change_branch_minimum_branch_number_set(cp, n);
}


bool
project_skip_unlucky_get(project_ty *pp)
{
    change::pointer cp = pp->change_get();
    return change_branch_skip_unlucky_get(cp);
}


void
project_skip_unlucky_set(project_ty *pp, bool n)
{
    change::pointer cp = pp->change_get();
    change_branch_skip_unlucky_set(cp, n);
}


int
project_name_ok(string_ty *s)
{
    const char      *sp;

    //
    // The horrible characters are file separators in a variety of
    // operating systems (unix, dos, mac, primos).
    //
    static char	    horrible[] =    "/\\:>";

    if (s->str_length == 0)
	return 0;
    for (sp = s->str_text; *sp; ++sp)
    {
	// C locale
	if
	(
	    !isprint((unsigned char)*sp)
	||
	    isspace((unsigned char)*sp)
	||
	    strchr(horrible, *sp)
	)
	    return 0;
    }
    return 1;
}


project::~project()
{
    assert(ref);
    if (ref)
    {
	project_free(ref);
	ref = 0;
    }
}


project::project(const nstring &arg) :
    ref(project_alloc(arg.get_ref()))
{
    assert(ref);
}


project::project(string_ty *arg) :
    ref(project_alloc(arg))
{
    assert(ref);
}


project::project(const project &arg) :
    ref(project_copy(arg.ref))
{
    assert(ref);
}


project &
project::operator=(const project &arg)
{
    assert(ref);
    assert(arg.ref);
    if (this != &arg && ref != arg.ref)
    {
	project_free(ref);
	ref = project_copy(arg.ref);
    }
    return *this;
}
