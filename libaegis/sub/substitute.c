/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate substitutes
 */

#include <sub.h>
#include <sub/substitute.h>
#include <trace.h>
#include <wstr_list.h>


static string_ty *gerr;
static sub_context_ty *gscp;


static void err _((const char *));

static void
err(message)
    const char	    *message;
{
    if (gerr)
	str_free(gerr);
    gerr = str_from_c(message);
    sub_context_error_set(gscp, gerr->str_text);
}


wstring_ty *
sub_substitute(scp, arg)
    sub_context_ty  *scp;
    wstring_list_ty *arg;
{
    string_ty	    *s;
    string_ty	    *lhs;
    string_ty	    *rhs;
    string_ty	    *rs;
    wstring_ty	    *ws;
    wstring_ty	    *result;

    /*
     * make sure there are enough arguments
     */
    trace(("sub_substitute()\n{\n"));
    if (arg->nitems < 3)
    {
	sub_context_error_set(scp, i18n("requires two or more arguments"));
	trace(("return NULL;\n"));
	trace(("}\n"));
	return 0;
    }
    lhs = wstr_to_str(arg->item[1]);
    rhs = wstr_to_str(arg->item[2]);

    /*
     * turn it into one big string to be substituted within
     */
    ws = wstring_list_to_wstring(arg, (size_t)3, arg->nitems, (char *)0);
    s = wstr_to_str(ws);
    wstr_free(ws);

    /*
     * do the substitution
     */
    gscp = scp;
    rs = str_re_substitute(lhs, rhs, s, err, 32767);
    gscp = 0;
    result = rs ? str_to_wstr(rs) : 0;

    /*
     * clean up and return
     */
    str_free(lhs);
    str_free(rhs);
    str_free(s);
    str_free(rs);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
