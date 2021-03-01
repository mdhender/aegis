/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to manipulate project state data
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <change.h>
#include <commit.h>
#include <error.h>
#include <gonzo.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <pstate.h>
#include <s-v-arg.h>
#include <trace.h>
#include <user.h>
#include <undo.h>


static void improve _((pstate));

static void
improve(pstate_data)
	pstate		pstate_data;
{
	trace(("improve(pstate_data = %08lX)\n{\n"/*}*/, pstate_data));
	if (!pstate_data->administrator)
		pstate_data->administrator =
			(pstate_administrator_list)
			pstate_administrator_list_type.alloc();
	if (!pstate_data->developer)
		pstate_data->developer =
			(pstate_developer_list)
			pstate_developer_list_type.alloc();
	if (!pstate_data->reviewer)
		pstate_data->reviewer =
			(pstate_reviewer_list)
			pstate_reviewer_list_type.alloc();
	if (!pstate_data->integrator)
		pstate_data->integrator =
			(pstate_integrator_list)
			pstate_integrator_list_type.alloc();
	if (!pstate_data->change)
		pstate_data->change =
			(pstate_change_list)
			pstate_change_list_type.alloc();
	if (!pstate_data->src)
		pstate_data->src =
			(pstate_src_list)
			pstate_src_list_type.alloc();
	if (!pstate_data->history)
		pstate_data->history =
			(pstate_history_list)
			pstate_history_list_type.alloc();
	if (!(pstate_data->mask & pstate_version_major_mask))
		pstate_data->version_major = 1;
	if (!(pstate_data->mask & pstate_version_minor_mask))
		pstate_data->version_minor = 0;
	/*
	 * owner: always read, always write, always search/exec.
	 * group: always read, never write, always search/exec.
	 * other: configurable read, never write, configurable search/exec.
	 *
	 * This means that the X's can be configured:
	 *	000 010 X1X
	 * and all else is pre-ordained.
	 */
	if (!pstate_data->umask)
		pstate_data->umask = DEFAULT_UMASK;
	pstate_data->umask = (pstate_data->umask & 5) | 022;
	trace((/*{*/"}\n"));
}


project_ty *
project_alloc(s)
	string_ty	*s;
{
	project_ty	*pp;

	trace(("project_alloc(s = \"%s\")\n{\n"/*}*/, s->str_text));
	pp = (project_ty *)mem_alloc_clear(sizeof(project_ty));
	pp->reference_count = 1;
	pp->name = str_copy(s);
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


void
project_free(pp)
	project_ty	*pp;
{
	trace(("project_free(pp = %08lX)\n{\n"/*}*/, pp));
	assert(pp->reference_count >= 1);
	pp->reference_count--;
	if (pp->reference_count <= 0)
	{
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

	n = lock_magic();
	if (pp->lock_magic == n)
		return;
	pp->lock_magic = n;

	if (!pp->pstate_data)
		return;
	if (pp->is_a_new_file)
		return;
	pstate_type.free(pp->pstate_data);
	pp->pstate_data = 0;
}


pstate
project_pstate_get(pp)
	project_ty	*pp;
{
	int		j, k;

	trace(("project_pstate_get(pp = %08lX)\n{\n"/*}*/, pp));
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
		os_become_orig();
		pp->pstate_data = pstate_read_file(path->str_text);
		os_become_undo();

		if (pp->pstate_data->next_change_number < 1)
		{
			project_fatal
			(
				pp,
				"%S: corrupted next_change_number field",
				pp->pstate_path
			);
		}
		if (pp->pstate_data->next_delta_number < 1)
		{
			project_fatal
			(
				pp,
				"%S: corrupted next_delta_number field",
				pp->pstate_path
			);
		}
		if (!pp->pstate_data->owner_name)
		{
			project_fatal
			(
				pp,
				"%S: corrupted owner_name field",
				pp->pstate_path
			);
		}
		if (!pp->pstate_data->group_name)
		{
			project_fatal
			(
				pp,
				"%S: corrupted group_name field",
				pp->pstate_path
			);
		}
		if (!pp->pstate_data->next_test_number)
		{
			project_fatal
			(
				pp,
				"%S: corrupted next_test_number field",
				pp->pstate_path
			);
		}
		improve(pp->pstate_data);

		for (j = 0; j < pp->pstate_data->src->length; ++j)
		{
			for (k = j + 1; k < pp->pstate_data->src->length; ++k)
			{
				if
				(
					str_equal
					(
				      pp->pstate_data->src->list[j]->file_name,
				      pp->pstate_data->src->list[k]->file_name
					)
				)
				{
					project_fatal
					(
						pp,
					       "%S: duplicate \"%S\" src entry",
						pp->pstate_path,
				       pp->pstate_data->src->list[j]->file_name
					);
				}

			}
		}
	}
	trace(("return %08lX;\n", pp->pstate_data));
	trace((/*{*/"}\n"));
	return pp->pstate_data;
}


void
project_pstate_lock_prepare(pp)
	project_ty	*pp;
{
	trace(("project_pstate_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	lock_prepare_pstate(pp->name);
	trace((/*{*/"}\n"));
}


void
project_build_read_lock_prepare(pp)
	project_ty	*pp;
{
	trace(("project_build_read_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	lock_prepare_build_read(pp->name);
	trace((/*{*/"}\n"));
}


void
project_build_write_lock_prepare(pp)
	project_ty	*pp;
{
	trace(("project_build_write_lock_prepare(pp = %08lX)\n{\n"/*}*/, pp));
	lock_prepare_build_write(pp->name);
	trace((/*{*/"}\n"));
}


void
project_bind_existing(pp)
	project_ty	*pp;
{
	string_ty	*s;

	/*
	 * make sure project exists
	 */
	trace(("project_bind_existing(pp = %08lX)\n{\n"/*}*/, pp));
	assert(!pp->home_path);
	s = gonzo_project_home_path_from_name(pp->name);
	if (!s)
		fatal("project \"%s\" unknown", pp->name->str_text);
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


void
project_bind_new(pp)
	project_ty	*pp;
{
	/*
	 * make sure name is appropriate length
	 */
	trace(("project_bind_new()\n{\n"/*}*/));
	if (pp->name->str_length > PATH_ELEMENT_MAX - 4)
	{
		fatal
		(
			"project name \"%s\" too long (by %d)",
			pp->name->str_text,
			pp->name->str_length - (PATH_ELEMENT_MAX - 4)
		);
	}

	/*
	 * make sure does not already exist
	 */
	if (gonzo_project_home_path_from_name(pp->name))
		fatal("project name \"%s\" already in use", pp->name->str_text);

	/*
	 * allocate data structures
	 */
	assert(!pp->pstate_data);
	assert(!pp->pstate_path);
	pp->is_a_new_file = 1;
	pp->pstate_data = (pstate)pstate_type.alloc();
	pp->pstate_data->next_change_number = 1;
	pp->pstate_data->next_delta_number = 1;
	pp->pstate_data->next_test_number = 1;
	pp->pstate_data->version_major = 1;
	improve(pp->pstate_data);
	trace((/*{*/"}\n"));
}


static int src_cmp _((const void *, const void *));

static int
src_cmp(s1p, s2p)
	const void	*s1p;
	const void	*s2p;
{
	pstate_src	s1;
	pstate_src	s2;

	s1 = *(pstate_src *)s1p;
	s2 = *(pstate_src *)s2p;
	return strcmp(s1->file_name->str_text, s2->file_name->str_text);
}


void
project_pstate_write(pp)
	project_ty	*pp;
{
	string_ty	*filename;
	string_ty	*filename_new;
	string_ty	*filename_old;
	static int	count;

	trace(("project_pstate_write(pp)\n{\n"/*}*/, pp));
	assert(pp->pstate_data);
	filename = project_pstate_path_get(pp);

	/*
	 * sort the file names
	 */
	assert(pp->pstate_data->src);
	if (pp->pstate_data->src->length >= 2)
	{
		assert(pp->pstate_data->src->list);
		qsort
		(
			pp->pstate_data->src->list,
			pp->pstate_data->src->length,
			sizeof(*pp->pstate_data->src->list),
			src_cmp
		);
	}

	/*
	 * write it out
	 */
	filename_new = str_format("%S,%d", filename, ++count);
	filename_old = str_format("%S,%d", filename, ++count);
	project_become(pp);
	if (pp->is_a_new_file)
	{
		undo_unlink_errok(filename_new);
		pstate_write_file(filename_new->str_text, pp->pstate_data);
		commit_rename(filename_new, filename);
	}
	else
	{
		undo_unlink_errok(filename_new);
		pstate_write_file(filename_new->str_text, pp->pstate_data);
		commit_rename(filename, filename_old);
		commit_rename(filename_new, filename);
		commit_unlink_errok(filename_old);
	}

	/*
	 * Change so the project owns it.
	 * (Only needed for new files, but be paranoid.)
	 */
	os_chmod(filename_new, 0644 & ~project_umask(pp));
	project_become_undo();
	str_free(filename_new);
	str_free(filename_old);
	trace((/*{*/"}\n"));
}


string_ty *
project_home_path_get(pp)
	project_ty	*pp;
{
	trace(("project_home_path_get(pp = %08lX)\n{\n"/*}*/, pp));
	if (!pp->home_path)
	{
		string_ty	*s;

		/*
		 * it is an error if the project name is not known
		 */
		s = gonzo_project_home_path_from_name(pp->name);
		if (!s)
			fatal("project \"%s\" unknown", pp->name->str_text);
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
	if (pp->home_path)
		fatal("duplicate -DIRectory option");
	pp->home_path = str_copy(s);
	trace((/*{*/"}\n"));
}


string_ty *
project_info_path_get(pp)
	project_ty	*pp;
{
	trace(("project_info_path_get(pp = %08lX)\n{\n"/*}*/, pp));
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
		pp->baseline_path_unresolved =
			str_format("%S/baseline", project_home_path_get(pp));
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


pstate_src
project_src_find(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	pstate		pstate_data;
	int		j;
	pstate_src	src_data;

	trace(("project_src_find(pp = %08lX, file_name = \"%s\")\n{\n"/*}*/, pp, file_name->str_text));
	pstate_data = project_pstate_get(pp);
	assert(pstate_data->src);
	for (j = 0; j < pstate_data->src->length; ++j)
	{
		src_data = pstate_data->src->list[j];
		if (str_equal(src_data->file_name, file_name))
			goto ret;
	}
	src_data = 0;
	ret:
	trace(("return %08lX;\n", src_data));
	trace((/*{*/"}\n"));
	return src_data;
}


pstate_src
project_src_new(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	pstate		pstate_data;
	pstate_src	src_data;
	pstate_src	*addr;
	type_ty		*type_p;

	trace(("project_src_new(pp = %08lX, file_name = \"%s\")\n{\n"/*}*/, pp, file_name->str_text));
	pstate_data = project_pstate_get(pp);
	assert(pstate_data->src);
	pstate_src_list_type.list_parse(pstate_data->src, &type_p, (void **)&addr);
	src_data = (pstate_src)pstate_src_type.alloc();
	*addr = src_data;
	src_data->file_name = str_copy(file_name);
	trace(("return %08lX;\n", src_data));
	trace((/*{*/"}\n"));
	return src_data;
}


void
project_src_remove(pp, file_name)
	project_ty	*pp;
	string_ty	*file_name;
{
	pstate		pstate_data;
	pstate_src	src_data;
	int		j;

	trace(("project_src_remove(pp = %08lX, file_name = \"%s\")\n{\n"/*}*/, pp, file_name->str_text));
	pstate_data = project_pstate_get(pp);
	assert(pstate_data->src);
	for (j = 0; ; ++j)
	{
		if (j >= pstate_data->src->length)
			goto ret;
		src_data = pstate_data->src->list[j];
		if (str_equal(src_data->file_name, file_name))
			break;
	}
	pstate_data->src->list[j] =
		pstate_data->src->list[--pstate_data->src->length];
	pstate_src_type.free(src_data);
	ret:
	trace((/*{*/"}\n"));
}


int
project_administrator_query(pp, user)
	project_ty	*pp;
	string_ty	*user;
{
	pstate		pstate_data;
	int		j;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->administrator);
	for (j = 0; j < pstate_data->administrator->length; ++j)
	{
		if (str_equal(user, pstate_data->administrator->list[j]))
			return 1;
	}
	return 0;
}


void
project_administrator_add(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	pstate		pstate_data;
	type_ty		*type_p;
	string_ty	**who_p;

	trace(("project_administrator_add(pp = %08lX, name = \"%s\")\n{\n"/*}*/, pp, name->str_text));
	pstate_data = project_pstate_get(pp);
	pstate_administrator_list_type.list_parse
	(
		pstate_data->administrator,
		&type_p,
		(void **)&who_p
	);
	*who_p = str_copy(name);
	trace((/*{*/"}\n"));
}


void
project_administrator_delete(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	size_t		k;
	pstate		pstate_data;

	pstate_data = project_pstate_get(pp);
	for (k = 0; k < pstate_data->administrator->length; ++k)
	{
		if (str_equal(name, pstate_data->administrator->list[k]))
		{
			str_free(pstate_data->administrator->list[k]);
			pstate_data->administrator->list[k] =
				pstate_data->administrator->list
				[
					--pstate_data->administrator->length
				];
			--k;
		}
	}
}


int
project_developer_query(pp, user)
	project_ty	*pp;
	string_ty	*user;
{
	pstate		pstate_data;
	size_t		j;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->developer);
	for (j = 0; j < pstate_data->developer->length; ++j)
	{
		if (str_equal(user, pstate_data->developer->list[j]))
			return 1;
	}
	return 0;
}


void
project_developer_add(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	pstate		pstate_data;
	type_ty		*type_p;
	string_ty	**who_p;

	pstate_data = project_pstate_get(pp);
	pstate_developer_list_type.list_parse
	(
		pstate_data->developer,
		&type_p,
		(void **)&who_p
	);
	*who_p = str_copy(name);
}


void
project_developer_delete(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	size_t		k;
	pstate		pstate_data;

	pstate_data = project_pstate_get(pp);
	for (k = 0; k < pstate_data->developer->length; ++k)
	{
		if (str_equal(name, pstate_data->developer->list[k]))
		{
			str_free(pstate_data->developer->list[k]);
			pstate_data->developer->list[k] =
				pstate_data->developer->list
				[
					--pstate_data->developer->length
				];
			--k;
		}
	}
}


int
project_integrator_query(pp, user)
	project_ty	*pp;
	string_ty	*user;
{
	pstate		pstate_data;
	size_t		j;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->integrator);
	for (j = 0; j < pstate_data->integrator->length; ++j)
	{
		if (str_equal(user, pstate_data->integrator->list[j]))
			return 1;
	}
	return 0;
}


void
project_integrator_add(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	pstate		pstate_data;
	type_ty		*type_p;
	string_ty	**who_p;

	pstate_data = project_pstate_get(pp);
	pstate_integrator_list_type.list_parse
	(
		pstate_data->integrator,
		&type_p,
		(void **)&who_p
	);
	*who_p = str_copy(name);
}


void
project_integrator_delete(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	size_t		k;
	pstate		pstate_data;

	pstate_data = project_pstate_get(pp);
	for (k = 0; k < pstate_data->integrator->length; ++k)
	{
		if (str_equal(name, pstate_data->integrator->list[k]))
		{
			str_free(pstate_data->integrator->list[k]);
			pstate_data->integrator->list[k] =
				pstate_data->integrator->list
				[
					--pstate_data->integrator->length
				];
			--k;
		}
	}
}


int
project_reviewer_query(pp, user)
	project_ty	*pp;
	string_ty	*user;
{
	pstate		pstate_data;
	size_t		j;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->reviewer);
	for (j = 0; j < pstate_data->reviewer->length; ++j)
	{
		if (str_equal(user, pstate_data->reviewer->list[j]))
			return 1;
	}
	return 0;
}


void
project_reviewer_add(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	pstate		pstate_data;
	type_ty		*type_p;
	string_ty	**who_p;

	pstate_data = project_pstate_get(pp);
	pstate_reviewer_list_type.list_parse
	(
		pstate_data->reviewer,
		&type_p,
		(void **)&who_p
	);
	*who_p = str_copy(name);
}


void
project_reviewer_delete(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	size_t		k;
	pstate		pstate_data;

	pstate_data = project_pstate_get(pp);
	for (k = 0; k < pstate_data->reviewer->length; ++k)
	{
		if (str_equal(name, pstate_data->reviewer->list[k]))
		{
			str_free(pstate_data->reviewer->list[k]);
			pstate_data->reviewer->list[k] =
				pstate_data->reviewer->list
				[
					--pstate_data->reviewer->length
				];
			--k;
		}
	}
}


pstate_history
project_history_new(pp)
	project_ty	*pp;
{
	pstate		pstate_data;
	pstate_history	history_data;
	pstate_history	*history_data_p;
	type_ty		*type_p;

	trace(("project_history_new()\n{\n"/*}*/));
	pstate_data = project_pstate_get(pp);
	assert(pstate_data->history);
	pstate_history_list_type.list_parse
	(
		pstate_data->history,
		&type_p,
		(void **)&history_data_p
	);
	history_data = (pstate_history)pstate_history_type.alloc();
	*history_data_p = history_data;
	trace(("return %08lX;\n", history_data));
	trace((/*{*/"}\n"));
	return history_data;
}


long
project_last_change_integrated(pp)
	project_ty	*pp;
{
	pstate		pstate_data;
	pstate_history	history_data;

	pstate_data = project_pstate_get(pp);
	if (!pstate_data->history || !pstate_data->history->length)
		return -1;
	history_data =
		pstate_data->history->list[pstate_data->history->length - 1];
	return history_data->change_number;
}


void
project_error(pp, s sva_last)
	project_ty	*pp;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*msg;

	sva_init(ap, s);
	msg = str_vformat(s, ap);
	va_end(ap);
	error
	(
		"project \"%s\": %s",
		project_name_get(pp)->str_text,
		msg->str_text
	);
	str_free(msg);
}


void
project_fatal(pp, s sva_last)
	project_ty	*pp;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*msg;

	sva_init(ap, s);
	msg = str_vformat(s, ap);
	va_end(ap);
	fatal
	(
		"project \"%s\": %s",
		project_name_get(pp)->str_text,
		msg->str_text
	);
}


void
project_verbose(pp, s sva_last)
	project_ty	*pp;
	char		*s;
	sva_last_decl
{
	va_list		ap;
	string_ty	*msg;

	sva_init(ap, s);
	msg = str_vformat(s, ap);
	va_end(ap);
	verbose
	(
		"project \"%s\": %s",
		project_name_get(pp)->str_text,
		msg->str_text
	);
	str_free(msg);
}


void
project_change_append(pp, cn)
	project_ty	*pp;
	long		cn;
{
	pstate		pstate_data;
	long		*change_p;
	type_ty		*type_p;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->change);
	pstate_change_list_type.list_parse
	(
		pstate_data->change,
		&type_p,
		(void **)&change_p
	);
	*change_p = cn;
}


void
project_change_delete(pp, cn)
	project_ty	*pp;
	long		cn;
{
	pstate		pstate_data;
	long		j, k;

	pstate_data = project_pstate_get(pp);
	assert(pstate_data->change);
	for (j = 0; j < pstate_data->change->length; ++j)
	{
		if (pstate_data->change->list[j] != cn)
			continue;
		for (k = j + 1; k < pstate_data->change->length; ++k)
			pstate_data->change->list[k - 1] =
				pstate_data->change->list[k];
		pstate_data->change->length--;
		break;
	}
}


string_ty *
project_version_get(pp)
	project_ty	*pp;
{
	string_ty	*s;
	pstate		pstate_data;
	pstate_history	history_data;

	trace(("project_version_get(pp = %08lX)\n{\n"/*}*/, pp));
	pstate_data = project_pstate_get(pp);
	assert(pstate_data->history);
	assert(pstate_data->history->length > 0);
	history_data =
		pstate_data->history->list[pstate_data->history->length - 1];
	s =
		str_format
		(
			"%ld.%ld.D%3.3ld",
			pstate_data->version_major,
			pstate_data->version_minor,
			history_data->delta_number
		);
	trace(("return \"%s\";\n", s->str_text));
	trace((/*{*/"}\n"));
	return s;
}


string_ty *
project_owner(pp)
	project_ty	*pp;
{
	return project_pstate_get(pp)->owner_name;
}


string_ty *
project_group(pp)
	project_ty	*pp;
{
	return project_pstate_get(pp)->group_name;
}


string_ty *
project_default_development_directory(pp)
	project_ty	*pp;
{
	pstate		pstate_data;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	pstate_data = project_pstate_get(pp);
	return pstate_data->default_development_directory;
}


user_ty *
project_user(pp)
	project_ty	*pp;
{
	user_ty		*up;

	trace(("project_user(pp = %08lX)\n{\n"/*}*/, pp));
	up = user_symbolic(pp, project_owner(pp));
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
project_umask(pp)
	project_ty	*pp;
{
	return project_pstate_get(pp)->umask;
}


int
project_delta_exists(pp, delta_number)
	project_ty	*pp;
	long		delta_number;
{
	int		result;
	pstate		pstate_data;
	long		j;
	pstate_history	history_data;

	trace(("project_delta_exists(pp = %08lX, delta_number = %ld)\n{\n"/*}*/, pp, delta_number));
	pstate_data = project_pstate_get(pp);
	if
	(
		pstate_data->history->length > 0
	&&
		pstate_data->history->list[pstate_data->history->length - 1]
			->delta_number == delta_number
	)
		result = -1;
	else
	{
		result = 0;
		for (j = 0; j < pstate_data->history->length; ++j)
		{
			history_data = pstate_data->history->list[j];
			if (history_data->delta_number == delta_number)
			{
				result = 1;
				break;
			}
		}
	}
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	project_delta_to_edit
 *
 * SYNOPSIS
 *	string_ty *project_delta_to_edit(project_ty *pp, long delta_number,
 *		string_ty *file_name);
 *
 * DESCRIPTION
 *	The project_delta_to_edit function is used to map a delta number
 *	into a specific edit number for a project source file.  This requires
 *	roll-forward of the edits to the named file, until the relevant
 *	delta is reached.
 *
 * ARGUMENTS
 *	pp		- project file is in
 *	delta_number	- delta number wanted
 *	file_name	- name of file
 *
 * RETURNS
 *	string_ty *;	string containing edit number,
 *			NULL if file does not exist at this delta.
 *
 * CAVEAT
 *	It is the caller's responsibility to free the string returned
 *	when not futher required.
 */

string_ty *
project_delta_to_edit(pp, delta_number, file_name)
	project_ty	*pp;
	long		delta_number;
	string_ty	*file_name;
{
	string_ty	*edit_number;
	pstate		pstate_data;
	pstate_history	history_data;
	long		j;
	change_ty	*cp;
	cstate_src	src_data;

	trace(("project_delta_exists(pp = %08lX, delta_number = %ld)\n{\n"/*}*/, pp, delta_number));
	edit_number = 0;
	pstate_data = project_pstate_get(pp);
	for (j = 0; j < pstate_data->history->length; ++j)
	{
		history_data = pstate_data->history->list[j];
		cp = change_alloc(pp, history_data->change_number);
		src_data = change_src_find(cp, file_name);
		if (src_data)
		{
			if (src_data->action == file_action_remove)
			{
				if (edit_number)
					str_free(edit_number);
				edit_number = 0;
			}
			else
			{
				/*
				 * there should always be an edit number,
				 * because it is checked for when the file
				 * is read in.
				 */
				assert(src_data->edit_number);
				if (edit_number)
					str_free(edit_number);
				edit_number = str_copy(src_data->edit_number);
			}
		}
		change_free(cp);
		if (history_data->delta_number == delta_number)
			break;
	}
	trace(("return %s;\n", edit_number ? edit_number->str_text : "NULL"));
	trace((/*{*/"}\n"));
	return edit_number;
}
