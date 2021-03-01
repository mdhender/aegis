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
 * MANIFEST: functions to manipulate checks
 */

#include <change/file.h>
#include <error.h> /* for assert */


void
change_file_fingerprint_check(change_ty *cp, fstate_src src_data)
{
	string_ty       *path;
	int		same;
	cstate		cstate_data;

	/*
	 * only useful in the 'being developed' and 'being integrated'
	 * states
	 */
	path = change_file_path(cp, src_data->file_name);
	assert(path);
	if (!src_data->file_fp)
		src_data->file_fp = fingerprint_type.alloc();
	assert(src_data->file_fp->youngest >= 0);
	assert(src_data->file_fp->oldest >= 0);
	change_become(cp);
	same = change_fingerprint_same(src_data->file_fp, path, 0);
	change_become_undo();
	assert(src_data->file_fp->youngest > 0);
	assert(src_data->file_fp->oldest > 0);

	/*
	 * if the file is unchanged, do nothing more
	 */
	if (same)
		return;

	/*
	 * nuke the difference time
	 */
	cstate_data = change_cstate_get(cp);
	if (cstate_data->state != cstate_state_being_integrated)
	{
		if (src_data->diff_file_fp)
		{
			fingerprint_type.free(src_data->diff_file_fp);
			src_data->diff_file_fp = 0;
		}
	}
	else
	{
#if 0
		if (src_data->idiff_file_fp)
		{
			fingerprint_type.free(src_data->idiff_file_fp);
			src_data->idiff_file_fp = 0;
		}
#endif
	}

	/*
	 * nuke the file's test times
	 */
	if (src_data->architecture_times)
	{
		fstate_src_architecture_times_list_type.free
		(
			src_data->architecture_times
		);
		src_data->architecture_times = 0;
	}
}
