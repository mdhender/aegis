/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to impliment the basename substitution
 */

#include <ac/string.h>

#include <str.h>
#include <sub.h>
#include <sub/basename.h>
#include <os.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_basename - the basename substitution
 *
 * SYNOPSIS
 *	wstring_ty *sub_basename(wstring_list_ty *arg);
 *
 * DESCRIPTION
 *	The sub_basename function implements the basename substitution.
 *	The basename substitution is replaced by the basename of
 *	the argument path, similar to the basename(1) command.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_basename(scp, arg)
    sub_context_ty  *scp;
    wstring_list_ty *arg;
{
    wstring_ty	    *result;
    string_ty	    *suffix;
    string_ty	    *s1;
    string_ty	    *s2;
    long	    len;

    trace(("sub_basename()\n{\n"));
    switch (arg->nitems)
    {
    default:
	sub_context_error_set(scp, i18n("requires one or two arguments"));
	result = 0;
	break;

    case 2:
	s1 = wstr_to_str(arg->item[1]);
	os_become_orig();
	s2 = os_entryname(s1);
	os_become_undo();
	str_free(s1);
	result = str_to_wstr(s2);
	str_free(s2);
	break;

    case 3:
	s1 = wstr_to_str(arg->item[1]);
	os_become_orig();
	s2 = os_entryname(s1);
	os_become_undo();
	str_free(s1);
	suffix = wstr_to_str(arg->item[2]);
	len = (long)s2->str_length - (long)suffix->str_length;
	if
	(
	    len > 0
	&&
	    !memcmp(s2->str_text + len, suffix->str_text, suffix->str_length)
	)
	    result = wstr_n_from_c(s2->str_text, len);
	else
	    result = str_to_wstr(s2);
	str_free(s2);
	str_free(suffix);
	break;
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
