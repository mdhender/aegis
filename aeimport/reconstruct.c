/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate reconstructs
 */

#include <change.h>
#include <change/file.h>
#include <commit.h>
#include <error.h>
#include <format.h>
#include <fstate.h>
#include <lock.h>
#include <change_set/list.h>
#include <os.h>
#include <project.h>
#include <project.h>
#include <project/file.h>
#include <project_hist.h>
#include <reconstruct.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>
#include <user.h>


static void str_reaper _((void *));

static void
str_reaper(p)
	void		*p;
{
	string_ty	*s;

	s = p;
	str_free(s);
}


static void walker _((symtab_ty *, string_ty *, void *, void *));

static void
walker(stp, filename, data, aux)
	symtab_ty	*stp;
	string_ty	*filename;
	void		*data;
	void		*aux;
{
	string_ty	*edit_number;
	project_ty	*pp;
	change_ty	*cp_bogus;
	string_ty	*bl;
	string_ty	*path;
	string_ty	*path_d;
	string_ty	*original;
	fstate_src	p_src_data;
	user_ty		*up;
	int		mode;
	fstate_src	src;

	/*
	 * Make a bogus change so that we can work with it.
	 */
	trace(("reconstruct::walker(filename = \"%s\")\n{\n",
		filename->str_text));
	pp = aux;
	cp_bogus = change_alloc(pp, project_next_change_number(pp, 1));
	change_bind_new(cp_bogus);
	change_architecture_from_pconf(cp_bogus);
	cp_bogus->bogus = 1;
	up = project_user(pp);

	bl = project_baseline_path_get(pp, 0);
	change_integration_directory_set(cp_bogus, bl);
	mode = 0755 & ~project_umask_get(pp);
	project_become(pp);
	os_mkdir_between(bl, filename, mode);
	project_become_undo();

	/*
	 * Extract the file
	 */
	edit_number = data;
	path = os_path_cat(bl, filename);
	trace_string(path->str_text);
	src = fstate_src_type.alloc();
	src->file_name = str_copy(filename);
	src->edit = history_version_type.alloc();
	src->edit->revision = str_copy(edit_number);
	change_run_history_get_command
	(
		cp_bogus,
		src,
		path,
		up
	);
	fstate_src_type.free(src);

	/*
	 * Fingerprint the file.
	 */
	p_src_data = project_file_find(pp, filename);
	assert(p_src_data);
	p_src_data->file_fp = fingerprint_type.alloc();
	project_become(pp);
	change_fingerprint_same(p_src_data->file_fp, path, 0);
	project_become_undo();

	/*
	 * Difference the file.
	 */
	path_d = str_format("%S,D", path);
	trace_string(path_d->str_text);
	original = str_from_c("/dev/null");
	change_run_diff_command(cp_bogus, up, original, path, path_d);
	str_free(original);

	/*
	 * Fingerprint the difference file.
	 */
	p_src_data->diff_file_fp = fingerprint_type.alloc();
	project_become(pp);
	change_fingerprint_same(p_src_data->diff_file_fp, path_d, 0);
	project_become_undo();

	str_free(path);
	str_free(path_d);
	change_free(cp_bogus);
	trace(("}\n"));
}


void
reconstruct(project_name, cslp)
	string_ty	*project_name;
	change_set_list_ty *cslp;
{
	project_ty	*pp;
	size_t		j;
	symtab_ty	*fvstp;

	/*
	 * Take some locks.
	 */
	trace(("reconstruct()\n{\n"));
	pp = project_alloc(project_name);
	project_bind_existing(pp);
	project_error(pp, 0, i18n("reconstruct baseline"));
	project_pstate_lock_prepare(pp);
	project_baseline_write_lock_prepare(pp);
	lock_take();

	/*
	 * Figure the file versions.
	 */
	fvstp = symtab_alloc(5);
	fvstp->reap = str_reaper;
	for (j = 0; j < cslp->length; ++j)
	{
		size_t		k;
		change_set_ty	*csp;

		csp = cslp->item[j];
		for (k = 0; k < csp->file.length; ++k)
		{
			change_set_file_ty *csfp;

			csfp = csp->file.item + k;
			symtab_assign
			(
				fvstp,
				csfp->filename,
				str_copy(csfp->edit)
			);
		}
	}

	/*
	 * Check out a copy of each file into the baseline,
	 * and produce a diff file to go with it.
	 */
	symtab_walk(fvstp, walker, pp);

	/*
	 * Write it all back out.
	 */
	project_pstate_write(pp);
	commit();
	lock_release();
	symtab_free(fvstp);
	project_verbose(pp, 0, i18n("import complete"));
	project_free(pp);
	trace(("}\n"));
}
