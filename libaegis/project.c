/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2001 Peter Miller;
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
 * MANIFEST: functions to manipulate project state data
 */

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/string.h>
#include <ac/stdlib.h>

#include <arglex2.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <commit.h>
#include <error.h>
#include <fstrcmp.h>
#include <gonzo.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <project_hist.h>
#include <pstate.h>
#include <skip_unlucky.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <undo.h>
#include <str_list.h>


static void convert_to_new_format _((project_ty *));

static void
convert_to_new_format(pp)
	project_ty	*pp;
{
	user_ty		*up;
	cstate_history	h;
	pstate		pstate_data;
	size_t		j;

	trace(("convert_to_new_format(pp = %08lX)\n{\n"/*}*/, (long)pp));
	pp->pcp = change_alloc(pp, TRUNK_CHANGE_NUMBER);
	change_bind_new(pp->pcp);
	change_branch_new(pp->pcp);

	/*
	 * The new change is in the 'being developed'
	 * state, and will be forever.
	 */
	up = user_executing(pp);
	h = change_history_new(pp->pcp, up);
	h->what = cstate_history_what_new_change;
	h = change_history_new(pp->pcp, up);
	h->what = cstate_history_what_develop_begin;
	user_free(up);
	pp->pcp->cstate_data->state = cstate_state_being_developed;

	/*
	 * set the development directory
	 */
	change_development_directory_set(pp->pcp, project_home_path_get(pp));

	/*
	 * Copy the information from the old format of project state
	 * into the newly created change.  Remember to clear the old
	 * stuff out, so that it isn't written back to the file.
	 */
	pstate_data = project_pstate_get(pp);

	change_branch_umask_set(pp->pcp, pstate_data->umask);
	pstate_data->umask = 0;

	if (pstate_data->owner_name)
	{
		/* this field is ignored */
		str_free(pstate_data->owner_name);
		pstate_data->owner_name = 0;
	}

	if (pstate_data->group_name)
	{
		/* this field is ignored */
		str_free(pstate_data->group_name);
		pstate_data->group_name = 0;
	}

	if (pstate_data->description)
	{
		project_description_set(pp, pstate_data->description);
		str_free(pstate_data->description);
		pstate_data->description = 0;
	}

	change_branch_developer_may_review_set
	(
		pp->pcp,
		pstate_data->developer_may_review
	);
	pstate_data->developer_may_review = 0;

	change_branch_developer_may_integrate_set
	(
		pp->pcp,
		pstate_data->developer_may_integrate
	);
	pstate_data->developer_may_integrate = 0;

	change_branch_reviewer_may_integrate_set
	(
		pp->pcp,
		pstate_data->reviewer_may_integrate
	);
	pstate_data->reviewer_may_integrate = 0;

	change_branch_developers_may_create_changes_set
	(
		pp->pcp,
		pstate_data->developers_may_create_changes
	);
	pstate_data->developers_may_create_changes = 0;

	change_branch_forced_develop_begin_notify_command_set
	(
		pp->pcp,
		pstate_data->forced_develop_begin_notify_command
	);
	pstate_data->forced_develop_begin_notify_command = 0;

	change_branch_develop_end_notify_command_set
	(
		pp->pcp,
		pstate_data->develop_end_notify_command
	);
	pstate_data->develop_end_notify_command = 0;

	change_branch_develop_end_undo_notify_command_set
	(
		pp->pcp,
		pstate_data->develop_end_undo_notify_command
	);
	pstate_data->develop_end_undo_notify_command = 0;

	change_branch_review_pass_notify_command_set
	(
		pp->pcp,
		pstate_data->review_pass_notify_command
	);
	pstate_data->review_pass_notify_command = 0;

	change_branch_review_pass_undo_notify_command_set
	(
		pp->pcp,
		pstate_data->review_pass_undo_notify_command
	);
	pstate_data->review_pass_undo_notify_command = 0;

	change_branch_review_fail_notify_command_set
	(
		pp->pcp,
		pstate_data->review_fail_notify_command
	);
	pstate_data->review_fail_notify_command = 0;

	change_branch_integrate_pass_notify_command_set
	(
		pp->pcp,
		pstate_data->integrate_pass_notify_command
	);
	pstate_data->integrate_pass_notify_command = 0;

	change_branch_integrate_fail_notify_command_set
	(
		pp->pcp,
		pstate_data->integrate_fail_notify_command
	);
	pstate_data->integrate_fail_notify_command = 0;

	change_branch_default_development_directory_set
	(
		pp->pcp,
		pstate_data->default_development_directory
	);
	pstate_data->default_development_directory = 0;

	change_branch_default_test_exemption_set
	(
		pp->pcp,
		pstate_data->default_test_exemption
	);
	pstate_data->default_test_exemption = 0;

	if (!pstate_data->copyright_years)
		pstate_data->copyright_years =
			pstate_copyright_years_list_type.alloc();
	for (j = 0; j < pstate_data->copyright_years->length; ++j)
	{
		change_copyright_year_append
		(
			pp->pcp,
			pstate_data->copyright_years->list[j]
		);
	}
	pstate_copyright_years_list_type.free(pstate_data->copyright_years);
	pstate_data->copyright_years = 0;

	/* no explict next change number in new format */
	pstate_data->next_change_number = 0;

	/* no explict next delta number in new format */
	pstate_data->next_delta_number = 0;

	if (!pstate_data->src)
		pstate_data->src = pstate_src_list_type.alloc();
	for (j = 0; j < pstate_data->src->length; ++j)
	{
		pstate_src	sp1;
		fstate_src	sp2;

		sp1 = pstate_data->src->list[j];
		if (!sp1->file_name)
		{
			sub_context_ty	*scp;

			yuck:
			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", pp->pstate_path);
			sub_var_set_charstar(scp, "FieLD_Name", "src");
			project_fatal(pp, scp, i18n("$filename: corrupted \"$field_name\" field"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		sp2 = change_file_new(pp->pcp, sp1->file_name);
		if (!(sp1->mask & pstate_src_usage_mask))
			goto yuck;
		sp2->usage = sp1->usage;
		sp2->mask |= fstate_src_usage_mask;
		sp2->action = file_action_create;
		sp2->mask |= fstate_src_action_mask;
		if (sp1->edit_number)
		{
			sp2->edit = history_version_type.alloc();
			sp2->edit->revision = str_copy(sp1->edit_number);
			sp2->edit_origin = history_version_type.alloc();
			sp2->edit_origin->revision = str_copy(sp1->edit_number);
		}
		sp2->locked_by = sp1->locked_by;
		sp2->about_to_be_created_by = sp1->about_to_be_created_by;
		sp2->deleted_by = sp1->deleted_by;
	}
	pstate_src_list_type.free(pstate_data->src);
	pp->pstate_data->src = 0;

	if (!pstate_data->history)
		pstate_data->history = pstate_history_list_type.alloc();
	for (j = 0; j < pstate_data->history->length; ++j)
	{
		pstate_history	hp;

		hp = pstate_data->history->list[j];
		change_branch_history_new
		(
			pp->pcp,
			hp->delta_number,
			hp->change_number
		);
	}
	pstate_history_list_type.free(pstate_data->history);
	pstate_data->history = 0;

	if (!pstate_data->change)
		pstate_data->change = pstate_change_list_type.alloc();
	for (j = 0; j < pstate_data->change->length; ++j)
	{
		change_branch_change_add
		(
			pp->pcp,
			pstate_data->change->list[j],
			0
		);
	}
	pstate_change_list_type.free(pstate_data->change);
	pstate_data->change = 0;

	/*
	 * copy the staff across
	 */
	pstate_data = project_pstate_get(pp);
	if (!pstate_data->administrator)
		pstate_data->administrator =
			pstate_administrator_list_type.alloc();
	for (j = 0; j < pstate_data->administrator->length; ++j)
	{
		change_branch_administrator_add
		(
			pp->pcp,
			pstate_data->administrator->list[j]
		);
	}
	pstate_administrator_list_type.free(pstate_data->administrator);
	pstate_data->administrator = 0;

	if (!pstate_data->developer)
		pstate_data->developer =
			pstate_developer_list_type.alloc();
	for (j = 0; j < pstate_data->developer->length; ++j)
	{
		change_branch_developer_add
		(
			pp->pcp,
			pstate_data->developer->list[j]
		);
	}
	pstate_developer_list_type.free(pstate_data->developer);
	pstate_data->developer = 0;

	if (!pstate_data->reviewer)
		pstate_data->reviewer =
			pstate_reviewer_list_type.alloc();
	for (j = 0; j < pstate_data->reviewer->length; ++j)
	{
		change_branch_reviewer_add
		(
			pp->pcp,
			pstate_data->reviewer->list[j]
		);
	}
	pstate_reviewer_list_type.free(pstate_data->reviewer);
	pstate_data->reviewer = 0;

	if (!pstate_data->integrator)
		pstate_data->integrator =
			pstate_integrator_list_type.alloc();
	for (j = 0; j < pstate_data->integrator->length; ++j)
	{
		change_branch_integrator_add
		(
			pp->pcp,
			pstate_data->integrator->list[j]
		);
	}
	pstate_integrator_list_type.free(pstate_data->integrator);
	pstate_data->integrator = 0;

	if (pstate_data->currently_integrating_change)
	{
		change_current_integration_set
		(
			pp->pcp,
			pstate_data->currently_integrating_change
		);
		pstate_data->currently_integrating_change = 0;
	}

	/*
	 * These should actually be acted on to create a change tree,
	 * but that will have to wait for a future aegis change, when
	 * branching is working properly.
	 *
	pstate_data->version_major = 0;
	pstate_data->version_minor = 0;
	 */

	if (pstate_data->version_previous)
	{
		pp->pcp->cstate_data->version_previous = pstate_data->version_previous;
		pstate_data->version_previous = 0;
	}

	/*
	 * By default we reuse change numbers.
	 */
	change_branch_reuse_change_numbers_set(pp->pcp, 1);

	/*
	 * Phew!  Who would ever have guessed there was so much to do
	 * when it came to converting a project to use branching?
	 */
	trace((/*{*/"}\n"));
}


project_ty *
project_alloc(s)
	string_ty	*s;
{
	project_ty	*pp;

	trace(("project_alloc(s = \"%s\")\n{\n"/*}*/, s->str_text));
	pp = (project_ty *)mem_alloc(sizeof(project_ty));
	pp->reference_count = 1;
	pp->name = str_copy(s);
	pp->home_path = 0;
	pp->baseline_path_unresolved = 0;
	pp->baseline_path = 0;
	pp->history_path = 0;
	pp->info_path = 0;
	pp->pstate_path = 0;
	pp->changes_path = 0;
	pp->pstate_data = 0;
	pp->is_a_new_file = 0;
	pp->lock_magic = 0;
	pp->pcp = 0;
	pp->uid = -1;
	pp->gid = -1;
	pp->parent = 0;
	pp->parent_bn = 0;
	pp->file_list = 0;
	trace(("return %08lX;\n", pp));
	trace((/*{*/"}\n"));
	return pp;
}


project_ty *
project_copy(pp)
	project_ty	*pp;
{
	trace(("project_copy(pp = %08lX)\n{\n"/*}*/, pp));
	assert(pp->reference_count >= 1);
	pp->reference_count++;
	trace(("return %08lX;\n", pp));
	trace((/*{*/"}\n"));
	return pp;
}


change_ty *
project_change_get(pp)
	project_ty	*pp;
{
	/*
	 * It could be an old project.  Make sure the pstate is read in,
	 * and converted if necessary.
	 */
	if (!pp->parent)
		project_pstate_get(pp);

	/*
	 * If the project change is not yet bound, bind to the trunk
	 * change.  It will already exist.
	 */
	if (!pp->pcp)
	{

		pp->pcp = change_alloc(pp, TRUNK_CHANGE_NUMBER);
		change_bind_existing(pp->pcp);
	}
	return pp->pcp;
}


void
project_free(pp)
	project_ty	*pp;
{
	trace(("project_free(pp = %08lX)\n{\n"/*}*/, pp));
	assert(pp->reference_count >= 1);
	pp->reference_count--;
	if (pp->reference_count <= 0)
	{
		if (pp->pcp)
			change_free(pp->pcp);
		assert(pp->name);
		str_free(pp->name);
		if (pp->home_path)
			str_free(pp->home_path);
		if (pp->baseline_path_unresolved)
			str_free(pp->baseline_path_unresolved);
		if (pp->baseline_path)
			str_free(pp->baseline_path);
		if (pp->history_path)
			str_free(pp->history_path);
		if (pp->info_path)
			str_free(pp->info_path);
		if (pp->pstate_path)
			str_free(pp->pstate_path);
		if (pp->changes_path)
			str_free(pp->changes_path);
		if (pp->pstate_data)
			pstate_type.free(pp->pstate_data);
		if (pp->parent)
			project_free(pp->parent);
		if (pp->file_list)
		{
			string_list_destructor(pp->file_list);
			mem_free(pp->file_list);
		}
		mem_free((char *)pp);
	}
	trace((/*{*/"}\n"));
}


static void lock_sync _((project_ty *));

static void
lock_sync(pp)
	project_ty	*pp;
{
	long		n;

	if (pp->parent)
		this_is_a_bug();
	n = lock_magic();
	if (pp->lock_magic == n)
		return;
	pp->lock_magic = n;

	if (pp->pstate_data && !pp->is_a_new_file)
	{
		pstate_type.free(pp->pstate_data);
		pp->pstate_data = 0;
	}
	if (pp->file_list)
	{
		string_list_destructor(pp->file_list);
		mem_free(pp->file_list);
		pp->file_list = 0;
	}
}


static void get_the_owner _((project_ty *));

static void
get_the_owner(pp)
	project_ty	*pp;
{
	string_ty	*s;

	/*
	 * If the project owner is already established,
	 * don't do anything here.
	 */
	if (pp->uid >= 0 && pp->gid >= 0)
		return;

	/*
	 * When checking the project home path owner, we append ``/.''
	 * to force the automounter to mount it.  Some automounters
	 * don't actually trigger until the directory lookup happens,
	 * which gives nasty results when we stat the directory to see
	 * who ownes it (the automounter frequently returns 0,0 which
	 * gives a "tampering" error).
	 */
	os_become_orig();
	s = str_format("%S/.", project_home_path_get(pp));
	os_owner_query(s, &pp->uid, &pp->gid);
	str_free(s);
	os_become_undo();

	/*
	 * Make sure the project UID and GID are acceptable.  This mirrors
	 * the tests in aegis/aenpr.c when the project is first created.
	 */
	if (pp->uid < AEGIS_MIN_UID)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", project_home_path_get(pp));
		sub_var_set_long(scp, "Number1", pp->uid);
		sub_var_set_long(scp, "Number2", AEGIS_MIN_UID);
		fatal_intl
		(
			scp,
		    i18n("$filename: uid $number1 invalid, must be >= $number2")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (pp->gid < AEGIS_MIN_GID)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", project_home_path_get(pp));
		sub_var_set_long(scp, "Number1", pp->gid);
		sub_var_set_long(scp, "Number2", AEGIS_MIN_GID);
		fatal_intl
		(
			scp,
		    i18n("$filename: gid $number1 invalid, must be >= $number2")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
}


pstate
project_pstate_get(pp)
	project_ty	*pp;
{
	trace(("project_pstate_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (pp->parent)
		this_is_a_bug();
	lock_sync(pp);
	if (!pp->pstate_data)
	{
		string_ty	*path;

		path = project_pstate_path_get(pp);
		pp->is_a_new_file = 0;

		/*
		 * can't become the project, because don't know who
		 * the project is, yet.
		 *
		 * This also means we can use UNIX system security
		 * to exclude unwelcome access.
		 */
		get_the_owner(pp);
		os_become_orig();
		os_chown_check(path, 0, pp->uid, pp->gid);
		pp->pstate_data = pstate_read_file(path);
		os_become_undo();

		if (!pp->pstate_data->next_test_number)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", pp->pstate_path);
			sub_var_set_charstar(scp, "FieLD_Name", "next_test_number");
			project_fatal
			(
				pp,
				scp,
				i18n("$filename: corrupted \"$field_name\" field")
			);
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		if (pp->pstate_data->next_change_number)
			convert_to_new_format(pp);
	}
	trace(("return %08lX;\n", pp->pstate_data));
	trace((/*{*/"}\n"));
	return pp->pstate_data;
}


static void waiting_for_lock _((void *));

static void
waiting_for_lock(p)
	void		*p;
{
	project_ty	*pp;

	pp = p;
	if (user_lock_wait(0))
		project_error(pp, 0, i18n("waiting for lock"));
	else
		project_fatal(pp, 0, i18n("lock not available"));
}


void
project_pstate_lock_prepare(pp)
	project_ty	*pp;
{
	trace(("project_pstate_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	if (pp->parent)
	{
		assert(pp->pcp);
		change_cstate_lock_prepare(pp->pcp);
	}
	else
		lock_prepare_pstate(pp->name, waiting_for_lock, pp);
	trace((/*{*/"}\n"));
}


void
project_pstate_lock_prepare_top(pp)
	project_ty	*pp;
{
	trace(("project_pstate_lock_prepare_top(pp = %08lX)\n{\n"/*}*/, pp));
	while (pp->parent)
		pp = pp->parent;
	project_pstate_lock_prepare(pp);
	trace((/*{*/"}\n"));
}


static void waiting_for_baseline_read_lock _((void *));

static void
waiting_for_baseline_read_lock(p)
	void		*p;
{
	project_ty	*pp;

	pp = p;
	if (user_lock_wait(0))
		project_error(pp, 0, i18n("waiting for baseline read lock"));
	else
		project_fatal(pp, 0, i18n("baseline read lock not available"));
}


void
project_baseline_read_lock_prepare(pp)
	project_ty	*pp;
{
	/*
	 * A branch's baseline is ``unioned'' with its parent's
	 * baseline, so we need to lock them as well - all the
	 * way up the tree.
	 */
	trace(("project_baseline_read_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	assert(pp);
	for (;;)
	{
		lock_prepare_baseline_read
		(
			pp->name,
			waiting_for_baseline_read_lock,
			pp
		);
		pp = pp->parent;
		if (!pp)
			break;
	}
	trace((/*{*/"}\n"));
}


static void waiting_for_baseline_write_lock _((void *));

static void
waiting_for_baseline_write_lock(p)
	void		*p;
{
	project_ty	*pp;

	pp = p;
	if (user_lock_wait(0))
		project_error(pp, 0, i18n("waiting for baseline write lock"));
	else
		project_fatal(pp, 0, i18n("baseline write lock not available"));
}


void
project_baseline_write_lock_prepare(pp)
	project_ty	*pp;
{
	trace(("project_baseline_write_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	lock_prepare_baseline_write
	(
		pp->name,
		waiting_for_baseline_write_lock,
		pp
	);
	trace((/*{*/"}\n"));
}


static void waiting_for_history_lock _((void *));

static void
waiting_for_history_lock(p)
	void		*p;
{
	project_ty	*pp;

	pp = p;
	if (user_lock_wait(0))
		project_error(pp, 0, i18n("waiting for history lock"));
	else
		project_fatal(pp, 0, i18n("history lock not available"));
}


void
project_history_lock_prepare(pp)
	project_ty	*pp;
{
	/*
	 * The history tool may have no locking of its own, and it will
	 * be ignored if it does.  Therefore, take the history lock in
	 * the deepest project.  This prevents aeip on different
	 * branches from trashing each other's history files.
	 */
	trace(("project_history_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	while (pp->parent)
		pp = pp->parent;
	lock_prepare_history(pp->name, waiting_for_history_lock, pp);
	trace((/*{*/"}\n"));
}


static int name_has_numeric_suffix _((string_ty *, string_ty **, long *));

static int
name_has_numeric_suffix(name, left, right)
	string_ty	*name;
	string_ty	**left;
	long		*right;
{
	char		*ep;

	ep = name->str_text + name->str_length;
	while (ep > name->str_text && isdigit((unsigned char)ep[-1]))
		--ep;
	if (!*ep)
		/* still pointing to end of string */
		return 0;
	if (ep < name->str_text + 2 || !ispunct((unsigned char)ep[-1]))
		return 0;
	*left = str_n_from_c(name->str_text, ep - 1 - name->str_text);
	*right = magic_zero_encode(atol(ep));
	return 1;
}


void
project_bind_existing(pp)
	project_ty	*pp;
{
	string_ty	*s;
	string_ty	*parent_name;
	int		alias_retry_count;

	/*
	 * make sure project exists
	 */
	trace(("project_bind_existing(pp = %08lX)\n{\n"/*}*/, pp));
	assert(!pp->home_path);
	alias_retry_count = 0;
	alias_retry:
	s = gonzo_project_home_path_from_name(pp->name);

	/*
	 * If the named project was not found, and it has a numeric suffix,
	 * then assume that it is a project.branch combination,
	 * and try to find the deeper project.
	 */
	if (!s && name_has_numeric_suffix(pp->name, &parent_name, &pp->parent_bn))
	{
		pp->parent = project_alloc(parent_name);
		project_bind_existing(pp->parent);
		pp->pcp = change_alloc(pp->parent, pp->parent_bn);
		change_bind_existing(pp->pcp);
		if (!change_was_a_branch(pp->pcp))
			change_fatal(pp->pcp, 0, i18n("not a branch"));

		/*
		 * rebuild the project name
		 *	...eventually, use the remembered punctuation
		 */
		str_free(pp->name);
		pp->name =
			str_format
			(
				"%S.%ld",
				project_name_get(pp->parent),
				magic_zero_decode(pp->parent_bn)
			);

		pp->changes_path =
			str_format
			(
				"%S.branch",
				project_change_path_get
				(
					pp->parent,
					pp->parent_bn
				)
			);
		return;
	}

	/*
	 * If the name was not found, try to find an alias match for it.
	 * Loop if an alias is found.
	 */
	if (!s)
	{
		string_ty	*other;

		other = gonzo_alias_to_actual(pp->name);
		if (other)
		{
			if (++alias_retry_count > 5)
			{
				project_fatal
				(
					pp,
					0,
					i18n("alias loop detected")
				);
			}
			str_free(pp->name);
			pp->name = str_copy(other);
			goto alias_retry;
		}
	}

	/*
	 * If the name was not found, try to find a fuzzy match for it.
	 * In general, this results in more informative error messages.
	 */
	if (!s)
	{
		string_list_ty		wl;
		string_ty	*best;
		double		best_weight;
		int		j;

		gonzo_project_list(&wl);
		best = 0;
		best_weight = 0.6;
		for (j = 0; j < wl.nstrings; ++j)
		{
			double		w;

			s = wl.string[j];
			w = fstrcmp(pp->name->str_text, s->str_text);
			if (w > best_weight)
			{
				best = s;
				best_weight = w;
			}
		}
		if (best)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", pp->name);
			sub_var_set_string(scp, "Guess", best);
			fatal_intl(scp, i18n("no $name project, closest is $guess"));
		}
		else
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", pp->name);
			fatal_intl(scp, i18n("no $name project"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
	}

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	pp->home_path = str_copy(s);
	trace((/*{*/"}\n"));
}


project_ty *
project_bind_branch(ppp, cp)
	project_ty	*ppp;
	change_ty	*cp;
{
	string_ty	*project_name;
	project_ty	*pp;

	trace(("project_bind_existing(ppp = %08lX, cp = %8.8lX)\n{\n"/*}*/,
		(long)ppp, (long)cp));
	/* punctuation? */
	project_name = str_format("%S.%ld", ppp->name, magic_zero_decode(cp->number));
	pp = project_alloc(project_name);
	pp->parent = project_copy(ppp);
	pp->parent_bn = cp->number;
	pp->pcp = cp;
	pp->changes_path =
		str_format
		(
			"%S.branch",
			project_change_path_get(ppp, cp->number)
		);
	trace(("return %8.8lX;\n", (long)pp));
	trace((/*{*/"}\n"));
	return pp;
}


void
project_bind_new(pp)
	project_ty	*pp;
{
	int		um;
	user_ty		*up;
	cstate_history	h;

	/*
	 * make sure does not already exist
	 */
	trace(("project_bind_new()\n{\n"/*}*/));
	if (gonzo_project_home_path_from_name(pp->name))
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", pp->name);
		fatal_intl(scp, i18n("project $name exists"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * allocate data structures
	 */
	assert(!pp->pstate_data);
	assert(!pp->pstate_path);
	pp->is_a_new_file = 1;
	pp->pstate_data = (pstate)pstate_type.alloc();
	pp->pstate_data->next_test_number = 1;
	os_become_orig_query(&pp->uid, &pp->gid, &um);

	pp->pcp = change_alloc(pp, TRUNK_CHANGE_NUMBER);
	change_bind_new(pp->pcp);
	change_branch_new(pp->pcp);
	project_umask_set(pp, um);

	/*
	 * The new change is in the 'being developed'
	 * state, and will be forever.
	 */
	up = user_executing(pp);
	h = change_history_new(pp->pcp, up);
	h->what = cstate_history_what_new_change;
	h = change_history_new(pp->pcp, up);
	h->what = cstate_history_what_develop_begin;
	user_free(up);
	pp->pcp->cstate_data->state = cstate_state_being_developed;

	trace((/*{*/"}\n"));
}


int
break_up_version_string(sp, buf, buflen_max, buflen_p, leading_punct)
	char		*sp;
	long		*buf;
	int		buflen_max;
	int		*buflen_p;
	int		leading_punct;
{
	int		buflen;
	long		n;

	if (*sp == 0)
		return -1;
	buflen = *buflen_p;
	for (;;)
	{
		/*
		 * one leading punctuation character
		 * but we don't care what it is (- and . are common)
		 * {C locale}
		 */
		if (leading_punct)
		{
			if (!ispunct((unsigned char)*sp))
				return -1;
			++sp;
		}
		else
			leading_punct = 1;

		/*
		 * the next one must be a digit
		 */
		if (!isdigit((unsigned char)*sp))
			return -1;

		/*
		 * The version string must not be too long.  (Even
		 * oracle, who use the most unbelievable 6 part version
		 * strings, will cope if you use a big enough buffer.)
		 */
		if (buflen >= buflen_max)
			return -1;

		/*
		 * collect the number
		 */
		n = 0;
		for (;;)
		{
			n = n * 10 + *sp++ - '0';
			if (!isdigit((unsigned char)*sp))
				break;
		}
		buf[buflen++] = magic_zero_encode(n);

		/*
		 * stop at end of string
		 * (note: trailing punctuation is illegal)
		 */
		if (!*sp)
			break;
	}
	*buflen_p = buflen;
	return 0;
}


void
extract_version_from_project_name(name_p, buf, buflen_max, buflen_p)
	string_ty	**name_p;
	long		*buf;
	int		buflen_max;
	int		*buflen_p;
{
	string_ty	*name;
	char		*sp;
	int		err;

	/*
	 * if the project name does not end in a digit,
	 * it can't end in a version string
	 */
	name = *name_p;
	if
	(
		name->str_length < 3
	||
		/* C locale */
		!isdigit((unsigned char)name->str_text[name->str_length - 1])
	)
		return;
	sp = name->str_text;

	/*
	 * move down the name, looking for a trailing version number
	 */
	for (; *sp; ++sp)
	{
		/*
		 * skip over leading non-punctuation
		 * {C locale}
		 */
		if (!ispunct((unsigned char)*sp))
			continue;

		/*
		 * attempt to break up the rest of the string
		 */
		err = break_up_version_string(sp, buf, buflen_max, buflen_p, 1);

		/*
		 * if there was an error, try again further down
		 */
		if (err)
			continue;

		/*
		 * the version number is now in the buffer,
		 * so shorten the name to match
		 */
		*name_p = str_n_from_c(name->str_text, sp - name->str_text);
		str_free(name);
		return;
	}
}


project_ty *
project_find_branch(pp, version_string)
	project_ty	*pp;
	char		*version_string;
{
	long		version[20];
	int		version_length;
	int		j;

	/*
	 * As a special case, the ``grandparent'' of a change is
	 * repesented by a branch name of ".." as this is expected to be
	 * the commonest variety of cross branch merge.
	 */
	if (!strcmp(version_string, "..") && pp->parent)
		return pp->parent;

	/*
	 * the version is relative to the trunk of the project
	 */
	while (pp->parent)
		pp = pp->parent;

	/*
	 * break the version string
	 */
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
		/* NOTREACHED */
		sub_context_delete(scp);
	}

	/*
	 * follow the branches down
	 */
	for (j = 0; j < version_length; ++j)
	{
		long		cn;
		change_ty	*cp;

		cn = version[j];
		cp = change_alloc(pp, cn);
		change_bind_existing(cp);
		if (!change_was_a_branch(cp))
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_charstar(scp, "Number", version_string);
			change_fatal(cp, scp, i18n("version $number not a branch"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}
		pp = project_bind_branch(pp, cp);
	}

	/*
	 * return the derived project
	 */
	return pp;
}


void
project_pstate_write(pp)
	project_ty	*pp;
{
	string_ty	*filename;
	string_ty	*filename_new;
	string_ty	*filename_old;
	static int	count;
	int		compress;

	trace(("project_pstate_write(pp)\n{\n"/*}*/, pp));

	/*
	 * write out the associated change, if it was read in
	 */
	if (pp->pcp)
		change_cstate_write(pp->pcp);

	/*
	 * write it out
	 */
	compress = project_compress_database_get(pp);
	if (pp->pstate_data)
	{
		filename = project_pstate_path_get(pp);
		filename_new = str_format("%S,%d", filename, ++count);
		filename_old = str_format("%S,%d", filename, ++count);
		project_become(pp);

		/* enums with 0 value not to be printed */
		type_enum_option_set();

		if (pp->is_a_new_file)
		{
			undo_unlink_errok(filename_new);
			pstate_write_file(filename_new, pp->pstate_data, compress);
			commit_rename(filename_new, filename);
		}
		else
		{
			undo_unlink_errok(filename_new);
			pstate_write_file(filename_new, pp->pstate_data, compress);
			commit_rename(filename, filename_old);
			commit_rename(filename_new, filename);
			commit_unlink_errok(filename_old);
		}

		/*
		 * Change so the project owns it.
		 * (Only needed for new files, but be paranoid.)
		 */
		os_chmod(filename_new, 0644 & ~project_umask_get(pp));
		project_become_undo();
		str_free(filename_new);
		str_free(filename_old);
	}
	trace((/*{*/"}\n"));
}


void
project_pstate_write_top(pp)
	project_ty	*pp;
{
	trace(("project_pstate_write_top(pp)\n{\n"/*}*/, pp));
	while (pp->parent)
		pp = pp->parent;
	project_pstate_write(pp);
	trace((/*{*/"}\n"));
}


string_ty *
project_home_path_get(pp)
	project_ty	*pp;
{
	trace(("project_home_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (pp->parent)
		this_is_a_bug();
	if (!pp->home_path)
	{
		string_ty	*s;

		/*
		 * it is an error if the project name is not known
		 */
		s = gonzo_project_home_path_from_name(pp->name);
		if (!s)
		{
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "Name", pp->name);
			fatal_intl(scp, i18n("no $name project"));
			/* NOTREACHED */
			sub_context_delete(scp);
		}

		/*
		 * To cope with automounters, directories are stored as given,
		 * or are derived from the home directory in the passwd file.
		 * Within aegis, pathnames have their symbolic links resolved,
		 * and any comparison of paths is done on this "system idea"
		 * of the pathname.
		 */
		pp->home_path = str_copy(s);
	}
	trace(("return \"%s\";\n", pp->home_path->str_text));
	trace((/*{*/"}\n"));
	return pp->home_path;
}


string_ty *
project_Home_path_get(pp)
	project_ty	*pp;
{
	while (pp->parent)
		pp = pp->parent;
	return project_home_path_get(pp);
}


void
project_home_path_set(pp, s)
	project_ty	*pp;
	string_ty	*s;
{
	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("project_home_path_set(pp = %08lX, s = \"%s\")\n{\n"/*}*/, pp, s->str_text));
	if (pp->parent)
		this_is_a_bug();
	if (pp->home_path)
	{
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "Name", arglex_token_name(arglex_token_directory));
		fatal_intl(scp, i18n("duplicate $name option"));
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	pp->home_path = str_copy(s);

	/*
	 * set it in the trunk change, too
	 */
	change_development_directory_set(project_change_get(pp), s);
	trace((/*{*/"}\n"));
}


string_ty *
project_top_path_get(pp, resolve)
	project_ty	*pp;
	int		resolve;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_top_path_get(cp, resolve);
}


string_ty *
project_info_path_get(pp)
	project_ty	*pp;
{
	trace(("project_info_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (pp->parent)
		this_is_a_bug();
	if (!pp->info_path)
	{
		pp->info_path =
			str_format("%S/info", project_home_path_get(pp));
	}
	trace(("return \"%s\";\n", pp->info_path->str_text));
	trace((/*{*/"}\n"));
	return pp->info_path;
}


string_ty *
project_changes_path_get(pp)
	project_ty	*pp;
{
	trace(("project_changes_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	assert(!pp->parent || pp->changes_path);
	if (!pp->changes_path)
	{
		pp->changes_path =
			str_format("%S/change", project_info_path_get(pp));
	}
	trace(("return \"%s\";\n", pp->changes_path->str_text));
	trace((/*{*/"}\n"));
	return pp->changes_path;
}


string_ty *
project_change_path_get(pp, n)
	project_ty	*pp;
	long		n;
{
	string_ty	*s;

	trace(("project_change_path_get(pp = %08lX, n = %ld)\n{\n"/*}*/, pp, n));
	n = magic_zero_decode(n);
	if (n == TRUNK_CHANGE_NUMBER)
		s = str_format("%S/trunk", project_info_path_get(pp));
	else
		s = str_format("%S/%d/%3.3d", project_changes_path_get(pp), n / 100, n);
	trace(("return \"%s\";\n", s->str_text));
	trace((/*{*/"}\n"));
	return s;
}


string_ty *
project_pstate_path_get(pp)
	project_ty	*pp;
{
	trace(("project_pstate_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (pp->parent)
		this_is_a_bug();
	if (!pp->pstate_path)
	{
		pp->pstate_path =
			str_format("%S/state", project_info_path_get(pp));
	}
	trace(("return \"%s\";\n", pp->pstate_path->str_text));
	trace((/*{*/"}\n"));
	return pp->pstate_path;
}


string_ty *
project_baseline_path_get(pp, resolve)
	project_ty	*pp;
	int		resolve;
{
	string_ty	*result;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("project_baseline_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (!pp->baseline_path_unresolved)
	{
		string_ty	*dd;

		dd = project_top_path_get(pp, 0);
		pp->baseline_path_unresolved = str_format("%S/baseline", dd);
	}
	if (!resolve)
		result = pp->baseline_path_unresolved;
	else
	{
		if (!pp->baseline_path)
		{
			project_become(pp);
			pp->baseline_path =
				os_pathname(pp->baseline_path_unresolved, 1);
			project_become_undo();
		}
		result = pp->baseline_path;
	}
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
project_history_path_get(pp)
	project_ty	*pp;
{
	trace(("project_history_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	while (pp->parent)
		pp = pp->parent;
	if (!pp->history_path)
	{
		pp->history_path =
			str_format("%S/history", project_home_path_get(pp));
	}
	trace(("return \"%s\";\n", pp->history_path->str_text));
	trace((/*{*/"}\n"));
	return pp->history_path;
}


string_ty *
project_name_get(pp)
	project_ty	*pp;
{
	trace(("project_name_get(pp = %08lX)\n{\n"/*}*/, pp));
	trace(("return \"%s\";\n", pp->name->str_text));
	trace((/*{*/"}\n"));
	return pp->name;
}


void
project_error(pp, scp, s)
	project_ty	*pp;
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	/*
	 * form the message
	 */
	subst_intl_project(scp, pp);
	msg = subst_intl(scp, s);

	/*
	 * pass the message to the error function
	 */
	/* re-use substitution context */
	sub_var_set_string(scp, "Message", msg);
	str_free(msg);
	subst_intl_project(scp, pp);
	error_intl(scp, i18n("project \"$project\": $message"));

	if (need_to_delete)
		sub_context_delete(scp);
}


void
project_fatal(pp, scp, s)
	project_ty	*pp;
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	/*
	 * form the message
	 */
	subst_intl_project(scp, pp);
	msg = subst_intl(scp, s);

	/*
	 * pass the message to the error function
	 */
	/* re-use substitution context */
	sub_var_set_string(scp, "Message", msg);
	str_free(msg);
	subst_intl_project(scp, pp);
	fatal_intl(scp, i18n("project \"$project\": $message"));
	/* NOTREACHED */

	if (need_to_delete)
		sub_context_delete(scp);
}


void
project_verbose(pp, scp, s)
	project_ty	*pp;
	sub_context_ty	*scp;
	char		*s;
{
	string_ty	*msg;
	int		need_to_delete;

	if (!scp)
	{
		scp = sub_context_new();
		need_to_delete = 1;
	}
	else
		need_to_delete = 0;

	/*
	 * form the message
	 */
	subst_intl_project(scp, pp);
	msg = subst_intl(scp, s);

	/*
	 * pass the message to the error function
	 */
	/* re-use the substitution context */
	sub_var_set_string(scp, "Message", msg);
	str_free(msg);
	subst_intl_project(scp, pp);
	verbose_intl(scp, i18n("project \"$project\": $message"));

	if (need_to_delete)
		sub_context_delete(scp);
}


void
project_change_append(pp, cn, is_a_branch)
	project_ty	*pp;
	long		cn;
	int		is_a_branch;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_branch_change_add(cp, cn, is_a_branch);
}


void
project_change_delete(pp, cn)
	project_ty	*pp;
	long		cn;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_branch_change_remove(cp, cn);
}


int
project_change_number_in_use(pp, cn)
	project_ty	*pp;
	long		cn;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_branch_change_number_in_use(cp, cn);
}


string_ty *
project_version_short_get(pp)
	project_ty	*pp;
{
	string_ty	*s;

	trace(("project_version_short_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (pp->parent)
	{
		assert(pp->parent_bn > 0 || pp->parent_bn == MAGIC_ZERO);
		s = project_version_short_get(pp->parent);
		if (s->str_length)
		{
			string_ty	*s2;

			/* ...punctuation? */
			s2 =
				str_format
				(
					"%S.%ld",
					s,
					magic_zero_decode(pp->parent_bn)
				);
			str_free(s);
			s = s2;
		}
		else
		{
			str_free(s);
			s = str_format("%ld", magic_zero_decode(pp->parent_bn));
		}
	}
	else
	{
		change_ty	*cp;
		pstate		pstate_data;

		cp = project_change_get(pp);
		pstate_data = project_pstate_get(pp);
		if (pstate_data->version_major || pstate_data->version_minor)
		{
			/*
			 * old style project, not yet branching
			 */
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
	trace((/*{*/"}\n"));
	return s;
}


string_ty *
project_version_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;
	long		dn;
	string_ty	*result;
	string_ty	*tmp;

	trace(("project_version_get(pp = %08lX)\n{\n"/*}*/, pp));
	cp = project_change_get(pp);
	dn = change_history_delta_latest(cp);
	tmp = project_version_short_get(pp);
	result = str_format("%S.D%3.3ld", tmp, dn);
	str_free(tmp);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


int
project_uid_get(pp)
	project_ty	*pp;
{
	while (pp->parent)
		pp = pp->parent;
	get_the_owner(pp);
	return pp->uid;
}


int
project_gid_get(pp)
	project_ty	*pp;
{
	while (pp->parent)
		pp = pp->parent;
	get_the_owner(pp);
	return pp->gid;
}


user_ty *
project_user(pp)
	project_ty	*pp;
{
	user_ty		*up;

	trace(("project_user(pp = %08lX)\n{\n"/*}*/, pp));
	while (pp->parent)
		pp = pp->parent;
	up = user_numeric(pp, project_uid_get(pp));
	trace(("return %08lX;\n", up));
	trace((/*{*/"}\n"));
	return up;
}


void
project_become(pp)
	project_ty	*pp;
{
	user_ty		*up;

	trace(("project_become(pp = %08lX)\n{\n"/*}*/, pp));
	up = project_user(pp);
	user_become(up);
	user_free(up);
	trace((/*{*/"}\n"));
}


void
project_become_undo()
{
	trace(("project_become_undo()\n{\n"/*}*/));
	user_become_undo();
	trace((/*{*/"}\n"));
}


int
project_is_readable(pp)
	project_ty	*pp;
{
	string_ty	*s;
	int		err;

	while (pp->parent)
		pp = pp->parent;
	s = project_pstate_path_get(pp);
	os_become_orig();
	err = os_readable(s);
	os_become_undo();
	return err;
}


long
project_next_test_number_get(pp)
	project_ty	*pp;
{
	pstate		pstate_data;
	long		result;
	int		skip;

	trace(("project_next_test_number_get(pp = %08lX)\n{\n"/*}*/, pp));
	skip = project_skip_unlucky_get(pp);
	while (pp->parent)
		pp = pp->parent;
	pstate_data = project_pstate_get(pp);
	result = pstate_data->next_test_number;
	if (skip)
		result = skip_unlucky(result);
	pstate_data->next_test_number = result + 1;
	trace(("return %ld;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


long
project_minimum_change_number_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_branch_minimum_change_number_get(cp);
}


void
project_minimum_change_number_set(pp, n)
	project_ty	*pp;
	long		n;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_branch_minimum_change_number_set(cp, n);
}


int
project_reuse_change_numbers_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_branch_reuse_change_numbers_get(cp);
}


void
project_reuse_change_numbers_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_branch_reuse_change_numbers_set(cp, n);
}


long
project_minimum_branch_number_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_branch_minimum_branch_number_get(cp);
}


void
project_minimum_branch_number_set(pp, n)
	project_ty	*pp;
	long		n;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_branch_minimum_branch_number_set(cp, n);
}


int
project_skip_unlucky_get(pp)
	project_ty	*pp;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	return change_branch_skip_unlucky_get(cp);
}


void
project_skip_unlucky_set(pp, n)
	project_ty	*pp;
	int		n;
{
	change_ty	*cp;

	cp = project_change_get(pp);
	change_branch_skip_unlucky_set(cp, n);
}


int
project_name_ok(s)
	string_ty	*s;
{
	char		*sp;

	/*
	 * The horrible characters are file separators in a variety of
	 * operating systems (unix, dos, mac, primos).
	 */
	static char	horrible[] = "/\\:>";

	if (s->str_length == 0)
		return 0;
	for (sp = s->str_text; *sp; ++sp)
	{
		/* C locale */
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
