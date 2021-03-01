/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate symlinkss
 */

#include <change.h>
#include <dir.h>
#include <error.h> /* for assert */
#include <gmatch.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


typedef struct slink_info_ty slink_info_ty;
struct slink_info_ty
{
	string_ty	*bl;
	string_ty	*dd;
	change_ty	*cp;
	pconf		pconf_data;
	int		minimum;
	user_ty		*up;
};


static void csltbl1 _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
csltbl1(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	slink_info_ty	*sip;
	string_ty	*s1;
	string_ty	*s2;
	long		j;
	pconf_symlink_exceptions_list lp;
	fstate_src	p_src;
	int		p_src_set;

	sip = p;
	s1 = os_below_dir(sip->bl, path);
	assert(s1);
	s2 = os_path_cat(sip->dd, s1);
	p_src = 0;
	p_src_set = 0;
	switch (msg)
	{
	case dir_walk_dir_before:
		if (!os_exists(s2))
			os_mkdir(s2, 02755);
		break;

	case dir_walk_dir_after:
		break;

	case dir_walk_special:
	case dir_walk_symlink:
		/*
		 * The minimum option says to only link project source
		 * files.  These two can't ever be source files.
		 */
		if (sip->minimum)
			break;
		/* fall through... */

	case dir_walk_file:
		/*
		 * The minimum option says to only link project source
		 * files.  This simulates -minimum integration builds.
		 */
		if (sip->minimum)
		{
			int	not_a_project_source_file;

			user_become_undo();
			p_src = project_file_find(sip->cp->pp, s1);
			p_src_set = 1;
			user_become(sip->up);
			not_a_project_source_file = !p_src;
			if (not_a_project_source_file)
				break;
		}

		/*
		 * If there is already something there, don't do anything.
		 * It doesn't matter whethyer it's a symlink or a file.
		 */
		if (os_exists(s2))
			break;

		/*
		 * avoid the symlink exceptions
		 */
		lp = sip->pconf_data->symlink_exceptions;
		assert(lp);
		for (j = 0; j < lp->length; ++j)
		{
			if (gmatch(lp->list[j]->str_text, s1->str_text))
				break;
		}
		if (j < lp->length)
			break;

		/*
		 * avoid removed files
		 */
		if (!p_src_set)
		{
			user_become_undo();
			p_src = project_file_find(sip->cp->pp, s1);
			p_src_set = 1;
			user_become(sip->up);
		}
		if (p_src && p_src->action == file_action_remove)
			break;

		/*
		 * make the symbolic link
		 */
		os_symlink(path, s2);
		break;
	}
	str_free(s1);
	str_free(s2);
}


static void csltbl2 _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
csltbl2(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	slink_info_ty	*sip;
	string_ty	*s1;

	/*
	 * walk the development directory,
	 * removing symlinks which point
	 * to non-existent files
	 */
	sip = p;
	switch (msg)
	{
	case dir_walk_dir_before:
	case dir_walk_dir_after:
	case dir_walk_file:
	case dir_walk_special:
		break;

	case dir_walk_symlink:
		/*
		 * If we are asked to do a minimum job, and the file is
		 * not a project source file, remove the symbolic link.
		 */
		if (sip->minimum)
		{
			int	not_a_project_source_file;

			s1 = os_below_dir(sip->bl, path);
			assert(s1);
			user_become_undo();
			not_a_project_source_file =
				!project_file_find(sip->cp->pp, s1);
			user_become(sip->up);
			if (not_a_project_source_file)
			{
				str_free(s1);
				os_unlink(path);
				break;
			}
			str_free(s1);
		}

		/*
		 * If the link points to a non-existent file, remove
		 * the link.
		 */
		s1 = os_readlink(path);
		if (s1->str_text[0] == '/' && !os_exists(s1))
			os_unlink(path);
		str_free(s1);
		break;
	}
}


void
change_create_symlinks_to_baseline(cp, pp, up, minimum)
	change_ty	*cp;
	project_ty	*pp;
	user_ty		*up;
	int		minimum;
{
	slink_info_ty	si;
	cstate		cstate_data;

	/*
	 * For each ancestor, create symlinks from the development
	 * directory to that ancestor's baseline if the file does not
	 * already exist.
	 */
	trace(("change_create_symlinks_to_baseline(cp = %8.8lX)\n{\n"/*}*/,
		(long)cp));
	assert(cp->reference_count >= 1);
	change_verbose(cp, 0, i18n("creating symbolic links to baseline"));
	cstate_data = change_cstate_get(cp);
	si.cp = cp;
	if (cstate_data->state == cstate_state_being_integrated)
		si.dd = change_integration_directory_get(cp, 0);
	else
		si.dd = change_development_directory_get(cp, 0);
	si.pconf_data = change_pconf_get(cp, 0);
	si.minimum = minimum;
	si.up = up;
	while (pp)
	{
		si.bl = project_baseline_path_get(pp, 0);
		user_become(up);
		dir_walk(si.bl, csltbl1, &si);
		user_become_undo();
		pp = pp->parent;
	}

	/*
	 * For each symlink in the development directory that points to
	 * a non-existent file, remove it.
	 */
	user_become(up);
	si.bl = si.dd;
	dir_walk(si.dd, csltbl2, &si);
	user_become_undo();
	trace((/*{*/"}\n"));
}


static void rsltbl _((void *, dir_walk_message_ty, string_ty *, struct stat *));

static void
rsltbl(p, msg, path, st)
	void		*p;
	dir_walk_message_ty msg;
	string_ty	*path;
	struct stat	*st;
{
	slink_info_ty	*sip;
	string_ty	*s1;
	string_ty	*s2;

	/*
	 * remove symlinks in the development directory
	 * which point to their counterpart in the baseline
	 */
	if (msg != dir_walk_symlink)
		return;
	sip = p;
	s1 = os_below_dir(sip->dd, path);
	s2 = os_path_cat(sip->bl, s1);
	str_free(s1);
	s1 = os_readlink(path);
	if (str_equal(s1, s2))
		os_unlink(path);
	str_free(s1);
	str_free(s2);
}


void
change_remove_symlinks_to_baseline(cp, pp, up)
	change_ty	*cp;
	project_ty	*pp;
	user_ty		*up;
{
	slink_info_ty	si;
	cstate		cstate_data;

	trace(("change_remove_symlinks_to_baseline(cp = %8.8lX)\n{\n"/*}*/,
		(long)cp));
	assert(cp->reference_count >= 1);
	change_verbose(cp, 0, i18n("removing symbolic links to baseline"));
	/*
	 * get the baseline now, so when get inside walk func,
	 * do not cause problem with multiple user-id setting.
	 */
	cstate_data = change_cstate_get(cp);
	si.cp = 0;
	if (cstate_data->state == cstate_state_being_integrated)
		si.dd = change_integration_directory_get(cp, 0);
	else
		si.dd = change_development_directory_get(cp, 0);
	si.pconf_data = 0;
	while (pp)
	{
		si.bl = project_baseline_path_get(pp, 0);
		user_become(up);
		dir_walk(si.dd, rsltbl, &si);
		user_become_undo();
		pp = pp->parent;
	}
	trace((/*{*/"}\n"));
}
