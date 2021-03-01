//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2004-2008 Peter Miller
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

#include <common/ac/string.h>

#include <aeimport/format/private.h>
#include <aeimport/format/sccs.h>
#include <aeimport/format/sccs/gram.h>
#include <aeimport/format/version.h>
#include <common/str_list.h>
#include <libaegis/os.h>


static void
destructor(format_ty *)
{
}


static int
is_a_candidate(format_ty *, string_ty *filename)
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
sanitize(format_ty *, string_ty *filename, int last_part)
{
    size_t	    j;

    //
    // Break the filename into path elements.
    //
    string_list_ty sl;
    sl.split(filename, "/");

    //
    // get rid of path elements named "SCCS"
    //
    string_list_ty sl2;
    for (j = 0; j < sl.nstrings; ++j)
    {
	string_ty	*s;

	s = sl.string[j];
	if (strcasecmp(s->str_text, "sccs"))
    	    sl2.push_back(s);
    }

    if (last_part)
    {
        //
        // Remove the "s." from the beginning of the last path element.
        //
        if (sl2.nstrings)
        {
            string_ty       *s;

            s = sl2.string[sl2.nstrings - 1];
            if
            (
                s->str_length > 2
            &&
                s->str_text[0] == 's'
            &&
                s->str_text[1] == '.'
            )
            {
                sl2.string[sl2.nstrings - 1] =
                    str_n_from_c(s->str_text + 2, s->str_length - 2);
                str_free(s);
            }
        }
    }

    //
    // Rebuild the filename from the remaining path elements.
    //
    return sl2.unsplit("/");
}


static format_version_ty *
read_versions(format_ty *, string_ty *physical, string_ty *logical)
{
    return sccs_parse(physical, logical);
}


static string_ty *
history_put(format_ty *)
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
history_get(format_ty *)
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
history_query(format_ty *)
{
    return
	str_from_c
	(
	    "sccs get -t -g ${quote ${dirname $history}/s.${basename $history}}"
	);
}


static string_ty *
diff(format_ty *)
{
    //
    // I'd prefer to say "diff -U10", but we can't rely on GNU
    // Diff being installed everywhere.  It's a risk even using
    // a context diff, because not all non-GNU diff progs have -c.
    //
    return
	str_from_c
	(
	    "set +e; "
	    CONF_DIFF " "
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
merge(format_ty *)
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
unlock(format_ty *, string_ty *filename)
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
