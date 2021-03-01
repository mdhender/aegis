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
 * MANIFEST: functions to manipulate rcss
 */

#include <format/private.h>
#include <format/rcs.h>
#include <format/rcs/gram.h>
#include <format/version.h>
#include <str_list.h>
#include <os.h>


static void
destructor(format_ty *fp)
{
}


static int
is_a_candidate(format_ty *fp, string_ty *filename)
{
    return
	(
	    filename->str_length > 2
	&&
	    filename->str_text[filename->str_length - 2] == ','
	&&
	    filename->str_text[filename->str_length - 1] == 'v'
	);
}


static string_ty *
sanitize(format_ty *fp, string_ty *filename)
{
    string_list_ty  sl;
    static string_ty *rcs =	    0;
    static string_ty *cvs =	    0;
    static string_ty *attic =	    0;
    size_t	    j;
    string_list_ty  sl2;

    if (!rcs)
    {
	rcs = str_from_c("RCS");
	cvs = str_from_c("CVS");
	attic = str_from_c("Attic");
    }

    /*
     * Break the filename into path elements.
     */
    string_list_constructor(&sl);
    str2wl(&sl, filename, "/", 0);

    /*
     * Get rid of path elements named "RCS" or "CVS" or "Attic".
     */
    string_list_constructor(&sl2);
    for (j = 0; j < sl.nstrings; ++j)
    {
	string_ty	*s;

	s = sl.string[j];
	if (!str_equal(s, rcs) && !str_equal(s, cvs) && !str_equal(s, attic))
	    string_list_append(&sl2, s);
    }
    string_list_destructor(&sl);

    /*
     * Remove the ",v" from the end of the last path element.
     */
    if (sl2.nstrings)
    {
	string_ty	*s;

	s = sl2.string[sl2.nstrings - 1];
	if
	(
	    s->str_length >= 2
	&&
	    s->str_text[s->str_length - 2] == ','
	&&
	    s->str_text[s->str_length - 1] == 'v'
	)
	{
	    sl2.string[sl2.nstrings - 1] =
		str_n_from_c(s->str_text, s->str_length - 2);
	    str_free(s);
	}
    }

    /*
     * Rebuild the filename from the remaining path elements.
     */
    filename = wl2str(&sl2, 0, sl2.nstrings, "/");
    string_list_destructor(&sl2);
    return filename;
}


static format_version_ty *
read_versions(format_ty *fp, string_ty *physical, string_ty *logical)
{
    return rcs_parse(physical, logical);
}


static string_ty *
history_put(format_ty *fp)
{
    return
	str_from_c
	(
	    "ci -u -d -M -m${quote ($version) ${change description}} "
		"-t/dev/null ${quote $input} ${quote $history,v}; "
	    "rcs -U ${quote $history,v}"
	);
}


static string_ty *
history_get(format_ty *fp)
{
    return
	str_from_c
	(
	    "co -r${quote $edit} -p ${quote $history,v} > ${quote $output}"
	);
}


static string_ty *
history_query(format_ty	*fp)
{
    return
	str_from_c
	(
	    "rlog -r ${quote $history,v} | awk '/^head:/ {print $$2}'"
	);
}


static string_ty *
diff(format_ty *fp)
{
    /*
     * I'd prefer to say ``diff -U10'', but we can't rely on GNU
     * Diff being installed everywhere.	 It's a risk even using
     * a context diff, because not all non-GNU diff progs have -c.
     */
    return
	str_from_c
	(
	    "set +e; "
	    "diff -c ${quote $original} ${quote $input} > ${quote $output}; "
	    "test $? -le 1"
	);
}


static string_ty *
merge(format_ty *fp)
{
    return
	str_from_c
	(
	    "set +e; "
	    "merge -p -L baseline -L C$c ${quote $mostrecent} "
		"${quote $original} ${quote $input} > ${quote $output}; "
	    "test $? -le 1"
	);
}


static void
unlock(format_ty *fp, string_ty *filename)
{
    int		    flags;
    string_ty	    *cmd;
    string_ty	    *qfn;

    /*
     * -e   means get rid of any access list
     * -ko  means no keyword expansion
     * -M   means do not send mail
     * -q   means operate quietly
     * -U   means set locking to non-strict
     *
     * There doesn't seem to be an option to get rid of all locks.
     */
    qfn = str_quote_shell(filename);
    cmd = str_format("rcs -e -M -q -U %S", qfn);
    str_free(qfn);
    flags = OS_EXEC_FLAG_ERROK;
    os_execute(cmd, flags, os_curdir());
    str_free(cmd);
}


static format_vtable_ty vtbl =
{
    sizeof(format_ty),
    destructor,
    is_a_candidate,
    sanitize,
    read_versions,
    history_put,
    history_get,
    history_query,
    diff,
    merge,
    unlock,
};


format_ty *
format_rcs_new()
{
    return format_new(&vtbl);
}
