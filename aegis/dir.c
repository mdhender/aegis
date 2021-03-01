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
 * MANIFEST: functions to walk directory trees
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <dir.h>
#include <error.h>
#include <glue.h>
#include <trace.h>
#include <word.h>


void
dir_walk(path, callback, arg)
	string_ty	*path;
	dir_walk_callback_ty callback;
	void		*arg;
{
	wlist		wl;
	struct stat	st;
	int		j;
	string_ty	*s;
	char		*cp;
	char		*data;
	long		data_len;

	trace(("dir_walk(path = %08lX, callback = %08lX, \
arg = %08lX)\n{\n"/*}*/, path, callback, arg));
	trace_string(path->str_text);
#ifdef S_IFLNK
	if (glue_lstat(path->str_text, &st))
		nfatal("lstat(\"%s\")", path->str_text);
#else
	if (glue_stat(path->str_text, &st))
		nfatal("stat(\"%s\")", path->str_text);
#endif
	switch (st.st_mode & S_IFMT)
	{
	case S_IFDIR:
		callback(arg, dir_walk_dir_before, path, &st);
		wl_zero(&wl);
		if (glue_read_whole_dir(path->str_text, &data, &data_len))
			nfatal("read \"%s\" directory", path->str_text);
		cp = data;
		while (cp < data + data_len)
		{
			s = str_from_c(cp);
			cp += s->str_length + 1;
			wl_append(&wl, s);
			str_free(s);
		}
		for (j = 0; j < wl.wl_nwords; ++j)
		{
			s = str_format("%S/%S", path, wl.wl_word[j]);
			dir_walk(s, callback, arg);
		}
		wl_free(&wl);
		callback(arg, dir_walk_dir_after, path, &st);
		break;

	case S_IFREG:
		callback(arg, dir_walk_file, path, &st);
		break;

#ifdef S_IFLNK
	case S_IFLNK:
		callback(arg, dir_walk_symlink, path, &st);
		break;
#endif

	default:
		callback(arg, dir_walk_special, path, &st);
		break;
	}
	trace((/*{*/"}\n"));
}
