//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate sccss
//

#include <ac/string.h>

#include <format/private.h>
#include <format/sccs.h>
#include <format/sccs/gram.h>
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
    char	    *cp;

    cp = strrchr(filename->str_text, '/');
    if (cp)
	++cp;
    else
	cp = filename->str_text;
    return (cp[0] == 's' && cp[1] == '.' && cp[2]);
}


static string_ty *
sanitize(format_ty *fp, string_ty *filename)
{
    string_list_ty  sl;
    size_t	    j;
    string_list_ty  sl2;

    //
    // Break the filename into path elements.
    //
    string_list_constructor(&sl);
    str2wl(&sl, filename, "/", 0);

    //
    // get rid of path elements named "SCCS"
    //
    string_list_constructor(&sl2);
    for (j = 0; j < sl.nstrings; ++j)
    {
	string_ty	*s;

	s = sl.string[j];
	if (strcasecmp(s->str_text, "sccs"))
    	    string_list_append(&sl2, s);
    }
    string_list_destructor(&sl);

    //
    // Remove the "s." from the beginning of the last path element.
    //
    if (sl2.nstrings)
    {
	string_ty       *s;

	s = sl2.string[sl2.nstrings - 1];
	if (s->str_length > 2 && s->str_text[0] == 's' && s->str_text[1] == '.')
	{
	    sl2.string[sl2.nstrings - 1] =
	       	str_n_from_c(s->str_text + 2, s->str_length - 2);
	    str_free(s);
	}
    }

    //
    // Rebuild the filename from the remaining path elements.
    //
    filename = wl2str(&sl2, 0, sl2.nstrings, "/");
    string_list_destructor(&sl2);
    return filename;
}


static format_version_ty *
read_versions(format_ty *fp, string_ty *physical, string_ty *logical)
{
    return sccs_parse(physical, logical);
}


static string_ty *
history_put(format_ty *fp)
{
    //
    // The ae-sccs-put script is distributed with Aegis.
    //
    return
	str_from_c
	(
	    "ae-sccs-put -G${quote $input} "
	    "-y${quote (${version}) ${change description}} "
	    "${quote ${dirname $history}/s.${basename $history}}");
}


static string_ty *
history_get(format_ty *fp)
{
    //
    // get -r<x>  get the specified version
    // get -G<x>  output to file <x>
    // get -s     work silently
    // get -k     no keyword expansion
    //
    return
	str_from_c
	(
	    "sccs get -r${quote $edit} -s -k -G${quote $output} "
	    "${quote ${dirname $history}/s.${basename $history}}"
	);
}


static string_ty *
history_query(format_ty *fp)
{
    return
	str_from_c
	(
	    "sccs get -t -g ${quote ${dirname $history}/s.${basename $history}}"
	);
}


static string_ty *
diff(format_ty *fp)
{
    //
    // I'd prefer to say ``diff -U10'', but we can't rely on GNU
    // Diff being installed everywhere.  It's a risk even using
    // a context diff, because not all non-GNU diff progs have -c.
    //
    return
	str_from_c
	(
	    "set +e; "
	    "diff "
#ifdef HAVE_GNU_DIFF
		"-U10 --text "
#else
		"-c "
#endif
		"${quote $original} ${quote $input} > ${quote $output}; "
	    "test $? -le 1"
	);
}


static string_ty *
merge(format_ty *fp)
{
    //
    // This is the RCS merge command.
    // SCCS doesn't have one of its own.
    //
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
    int	            flags;
    string_ty	    *cmd;
    string_ty	    *qfn;

    //
    // -di	no keyword expansion
    //
    qfn = str_quote_shell(filename);
    cmd = str_format("sccs admin -di %s", qfn->str_text);
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
format_sccs_new(void)
{
    return format_new(&vtbl);
}
