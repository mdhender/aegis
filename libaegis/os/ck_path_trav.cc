//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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
//
#include <common/ac/errno.h>
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/trace.h>
#include <libaegis/glue.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


static string_ty *
mode_string(int mode)
{
    char flags[9];

    flags[0] = (mode & 0400) ? 'r' : '-';
    flags[1] = (mode & 0200) ? 'w' : '-';
    flags[2] =
	(
	    (mode & 04000)
	?
	    ((mode & 0100) ? 's' : 'S')
	:
	    ((mode & 0100) ? 'x' : '-')
	);

    flags[3] = (mode & 040) ? 'r' : '-';
    flags[4] = (mode & 020) ? 'w' : '-';
    flags[5] =
	(
	    (mode & 02000)
	?
	    ((mode & 010) ? 's' : 'S')
	:
	    ((mode & 010) ? 'x' : '-')
	);

    flags[6] = (mode & 4) ? 'r' : '-';
    flags[7] = (mode & 2) ? 'w' : '-';
    flags[8] =
	(
	    (mode & 01000)
	?
	    ((mode & 1) ? 't' : 'T')
	:
	    ((mode & 1) ? 'x' : '-')
	);

    return str_n_from_c(flags, 9);
}


void
os_check_path_traversable(string_ty *path)
{
    trace(("os_check_path_traversable(path = \"%s\")\n{\n", path->str_text));
    os_become_must_be_active();
    if (path->str_length > 1)
    {
	string_ty *dir = os_dirname_relative(path);
	os_check_path_traversable(dir);
	str_free(dir);
    }

    struct stat st;
    int err = glue_stat(path->str_text, &st);
    if (err < 0)
    {
	int errno_old = errno;
	if (errno_old == ENOENT)
	{
	    trace(("}\n"));
	    return;
	}
	sub_context_ty sc;
	sc.errno_setx(errno_old);
	sc.var_set_string("File_Name", path);
	sc.fatal_intl(i18n("stat $filename: $errno"));
	// NOTREACHED
    }

    int uid = 13;
    int gid = 13;
    os_become_query(&uid, &gid, 0);

    int need = 0111;
    const char *mode_change = "og=u-w";
    if ((int)st.st_uid == uid)
    {
	need = 0100;
	mode_change = "u+x";
    }
    else if ((int)st.st_gid == gid)
    {
	need = 010;
	mode_change = "g+x";
    }
    else
    {
	need = 1;
	mode_change = "o+x";
    }
    if ((st.st_mode & need) == 0)
    {
	sub_context_ty sc;
	sc.var_set_string("File_Name", path);
	string_ty *mode_old = mode_string(st.st_mode & 07777);
	sc.var_set_string("Mode_Old", mode_old);
	sc.var_optional("Mode_Old");
	str_free(mode_old);
	string_ty *mode_new = mode_string((st.st_mode & 07777) | need);
	sc.var_set_string("Mode_New", mode_new);
	sc.var_optional("Mode_New");
	str_free(mode_new);
	sc.var_set_charstar("Mode_Change", mode_change);
	sc.fatal_intl(i18n("$filename needs permissions $mode_change"));
	// NOTREACHED
    }
    trace(("}\n"));
}
