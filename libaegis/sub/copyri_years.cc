//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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
// MANIFEST: functions to manipulate copyri_yearss
//

#include <change/branch.h>
#include <error.h> // for assert
#include <project/history.h>
#include <str_list.h>
#include <sub/copyri_years.h>
#include <sub.h>
#include <trace.h>
#include <wstr.h>
#include <wstr_list.h>


static int
icmp(const void *va, const void *vb)
{
    const int	    *a;
    const int	    *b;

    a = (const int *)va;
    b = (const int *)vb;
    if (*a == *b)
	return 0;
    if (*a < *b)
	return -1;
    return 1;
}


//
// NAME
//	sub_copyright_years - the change substitution
//
// SYNOPSIS
//	wstring_ty *sub_copyright_years(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_copyright_years function implements the change
//	substitution.  The copyright_years substitution is replaced by
//	the range of copyright years in the project's state, and
//	maintained at integrate begin time.  Do not use this to insert
//	into new files, it is not guaranteed to be up-to-date until the
//	integrate build, use ${date %Y} instead.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_copyright_years(sub_context_ty *scp, wstring_list_ty *arg)
{
    wstring_ty	    *result;
    int		    ary[100]; // a century should be enough  :-)
    int		    ary_len;
    int		    ary_len2;
    string_list_ty  wl;
    int 	    j;
    string_ty	    *s;
    change_ty	    *cp;

    trace(("sub_copyright_years()\n{\n"));
    result = 0;
    if (arg->nitems != 1)
    {
	sub_context_error_set(scp, i18n("requires zero arguments"));
	goto done;
    }
    cp = sub_context_change_get(scp);
    if (!cp || cp->bogus)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	goto done;
    }

    //
    // Extract the copyright years from the project
    // and the change.	Don't worry about duplicates.
    //
    project_copyright_years_get(cp->pp, ary, SIZEOF(ary), &ary_len);
    change_copyright_years_get
    (
	cp,
	ary + ary_len,
	(size_t)(SIZEOF(ary) - ary_len),
	&ary_len2
    );
    ary_len += ary_len2;

    //
    // sort the array
    //
    qsort(ary, (size_t)ary_len, sizeof(ary[0]), icmp);

    //
    // build the text string for the result
    // this is where duplicates are removed
    //
    string_list_constructor(&wl);
    for (j = 0; j < ary_len; ++j)
    {
	if (j && ary[j - 1] == ary[j])
	    continue;
	s = str_format("%d", ary[j]);
	string_list_append(&wl, s);
	str_free(s);
    }
    s = wl2str(&wl, 0, (int)wl.nstrings, ", ");
    string_list_destructor(&wl);
    result = str_to_wstr(s);
    str_free(s);

    //
    // here for all exits
    //
    done:
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}