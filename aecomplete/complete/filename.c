/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate filenames
 */

#include <ac/stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <complete/filename.h>
#include <complete/private.h>
#include <glue.h>
#include <os.h>
#include <str_list.h>
#include <shell.h>


typedef struct complete_filename_ty complete_filename_ty;
struct complete_filename_ty
{
    complete_ty     inherited;
    int             regular_ok;
    int             special_ok;
};


static void destructor _((complete_ty *));

static void
destructor(cp)
    complete_ty     *cp;
{
    complete_filename_ty *this;

    this = (complete_filename_ty *)cp;
}


static void read_dir _((string_ty *, string_list_ty *));

static void
read_dir(path, wlp)
    string_ty       *path;
    string_list_ty  *wlp;
{
    char            *data;
    long            data_len;
    char            *cp;
    string_ty       *s;

    string_list_constructor(wlp);
    if (glue_read_whole_dir(path->str_text, &data, &data_len))
    {
	return;
    }
    cp = data;
    while (cp < data + data_len)
    {
	s = str_from_c(cp);
	cp += s->str_length + 1;
	string_list_append(wlp, s);
	str_free(s);
    }
}


static void perform _((complete_ty *, shell_ty *));

static void
perform(cp, sh)
    complete_ty     *cp;
    shell_ty        *sh;
{
    complete_filename_ty *this;
    string_ty       *prefix;
    string_ty       *prefix_dir;
    string_ty       *prefix_ent;
    size_t          j;
    string_list_ty  names;

    this = (complete_filename_ty *)cp;
    prefix = shell_prefix_get(sh);

    /*
     * Break the prefix into directory part and filename part.
     */
    prefix_dir = os_dirname_relative(prefix);
    prefix_ent = os_entryname_relative(prefix);

    os_become_orig();
    read_dir(prefix_dir, &names);
    os_become_undo();

    for (j = 0; j < names.nstrings; ++j)
    {
	string_ty       *name;
	int             err;
	struct stat     st;
	string_ty       *path;
	static struct stat stz;

	name = names.string[j];
	if (!str_leading_prefix(name, prefix_ent))
	    continue;
	path = os_path_cat(prefix_dir, name);
#ifdef S_IFLNK
	err = glue_lstat(path->str_text, &st);
#else
	err = glue_stat(path->str_text, &st);
#endif
	if (err)
	    st = stz;
	if (S_ISDIR(st.st_mode))
	{
	    string_ty       *s;

	    /*
	     * We emit directories with and without a slash, so that Bash
	     * doesn't thing we are finished, and put a space after the
	     * argument if there is only one directory in the answer.
	     */
	    shell_emit(sh, path);
	    s = str_format("%S/", path);
	    shell_emit(sh, s);
	    str_free(s);
	}
	else if (S_ISREG(st.st_mode))
	{
	    if (this->regular_ok)
		shell_emit(sh, path);
	}
	else
	{
	    if (this->special_ok)
		shell_emit(sh, path);
	}
	str_free(path);
    }
    string_list_destructor(&names);
    str_free(prefix_ent);
    str_free(prefix_dir);
}


static complete_vtbl_ty vtbl =
{
    destructor,
    perform,
    sizeof(complete_filename_ty),
    "filename",
};


complete_ty *
complete_filename(dir_only)
    int             dir_only;
{
    complete_ty     *result;
    complete_filename_ty *this;

    result = complete_new(&vtbl);
    this = (complete_filename_ty *)result;
    this->regular_ok = !dir_only;
    this->special_ok = 0;
    return result;
}