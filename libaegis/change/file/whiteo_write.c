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
 * MANIFEST: functions to manipulate whiteo_writes
 */

#include <change.h>
#include <error.h> /* for assert */
#include <file.h>
#include <os.h>
#include <project/file.h>
#include <undo.h>
#include <user.h>


void
change_file_whiteout_write(cp, filename, up)
	change_ty	*cp;
	string_ty	*filename;
	user_ty		*up;
{
	string_ty	*dd;
	string_ty	*s2;

	dd = change_development_directory_get(cp, 0);

	/*
	 * Remove any existing file (this cleans up junk, and
	 * breaks the link if we are using symlink trees).
	 */
	s2 = str_format("%S/%S", dd, filename);
	user_become(up);
	if (os_exists(s2))
		os_unlink(s2);
	user_become_undo();

	if (user_whiteout(up))
	{
		string_ty	*content;

		content = change_file_whiteout(cp, filename);
		if (content)
		{
			int		umask;
			int		mode;
			fstate_src	p_src_data;

			/*
			 * create a junk file in the change
			 */
			umask = ~change_umask(cp);
			user_become(up);
			os_mkdir_between(dd, filename, 02755);
			undo_unlink_errok(s2);
			mode = 0644 & ~umask;
			file_from_string(s2, content, mode);
			str_free(content);
			user_become_undo();

			/*
			 * update the mod-time to match the project
			 * (if exists in project)
			 */
			p_src_data = project_file_find(cp->pp, filename);
			if
			(
				p_src_data
			&&
				(
					p_src_data->deleted_by
				||
					p_src_data->about_to_be_created_by
				)
			)
				p_src_data = 0;
			if (p_src_data)
			{
				string_ty	*s3;
				time_t		mtime_old;
				time_t		mtime_young;
				time_t		now;

				/*
				 * Make sure the whiteout file is
				 * younger than the baseline file.
				 * This ensures that a build (which
				 * will fail) is triggered if the file
				 * is still being referred to.
				 */
				time(&now);
				s3 = project_file_path(cp->pp, filename);
				assert(s3);
				user_become(up);
				os_mtime_range(s3, &mtime_old, &mtime_young);
				str_free(s3);
				if (now <= mtime_old)
					now = mtime_old + 60;
				if (now <= mtime_young)
					now = mtime_young + 60;
				os_mtime_set_errok(s2, now);
				user_become_undo();
			}
		}
	}
	str_free(s2);
}
