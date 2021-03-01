//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
// MANIFEST: implementation of the sub_email_addres class
//

#include <common/arglex.h>
#include <common/nstring.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/email_addres.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <common/wstr/list.h>


wstring_ty *
sub_email_address(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_email_address()\n{\n"));
    project_ty *pp = sub_context_project_get(scp);
    string_list_ty words;
    bool quote = false;
    const char *separator = 0;
    size_t j = 1;
    for (; j < arg->size(); ++j)
    {
	nstring option(wstr_to_str(arg->get(j)));
	if (arglex_compare("-Comma", option.c_str(), 0))
	    separator = ", ";
	else if (arglex_compare("-Quote", option.c_str(), 0))
	    quote = true;
	else
	    break;
    }
    for (; j < arg->size(); ++j)
    {
	nstring login(wstr_to_str(arg->get(j)));
	user_ty *up = user_symbolic(pp, login.get_ref());
	string_ty *s = user_email_address(up);
	if (quote)
	{
	    string_ty *s2 = str_quote_shell(s);
	    words.push_back_unique(s2);
	    str_free(s2);
	}
	else
	    words.push_back_unique(s);
	// do not str_free(s) it's cached
	user_free(up);
    }

    //
    // Convert the work list to a single string.
    //
    nstring rs(words.unsplit(separator));
    wstring_ty *result = str_to_wstr(rs.get_ref());

    //
    // all done
    //
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
