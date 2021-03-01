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
 * MANIFEST: functions to manipulate rmdirs
 */

#include <ac/errno.h>
#include <ac/stddef.h>
#include <ac/stdlib.h>
#include <ac/unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dir.h>
#include <error.h>
#include <glue.h>
#include <lock.h>
#include <os.h>
#include <sub.h>
#include <undo.h>


static void rmdir_callback _((void *, dir_walk_message_ty, string_ty *,
	struct stat *));

static void
rmdir_callback(arg, message, path, st)
	void		*arg;
	dir_walk_message_ty message;
	string_ty	*path;
	struct stat	*st;
{
	switch (message)
	{
	case dir_walk_dir_before:
		if (!(st->st_mode & 0200))
			chmod(path->str_text, ((st->st_mode & 07777) | 0200));
		break;

	case dir_walk_dir_after:
		if (rmdir(path->str_text))
		{
			sub_context_ty	*scp;

			if (errno == ENOENT)
				break;
			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set_string(scp, "File_Name", path);
			error_intl(scp, i18n("warning: rmdir $filename: $errno"));
			sub_context_delete(scp);
		}
		break;

	case dir_walk_file:
	case dir_walk_special:
	case dir_walk_symlink:
		if (unlink(path->str_text))
		{
			sub_context_ty	*scp;

			if (errno == ENOENT)
				break;
			scp = sub_context_new();
			sub_errno_set(scp);
			sub_var_set_string(scp, "File_Name", path);
			error_intl(scp, i18n("warning: unlink $filename: $errno"));
			sub_context_delete(scp);
		}
		break;
	}
}


int
rmdir_bg(path)
	char		*path;
{
	string_ty	*s;

	switch (fork())
	{
	case -1:
		nfatal("fork");

	case 0:
		/* child */
		os_interrupt_ignore();
		lock_release_child(); /* don't hold locks */
		undo_cancel(); /* don't do anything else! */
		s = str_from_c(path);
		dir_walk(s, rmdir_callback, 0);
		str_free(s);
		exit(0);

	default:
		/* parent */
		break;
	}
	return 0;
}


int
rmdir_tree(path)
	char		*path;
{
	string_ty	*s;

	s = str_from_c(path);
	dir_walk(s, rmdir_callback, 0);
	str_free(s);
	return 0;
}
