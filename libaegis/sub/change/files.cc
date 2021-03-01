//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate filess
//

#include <common/ac/string.h>

#include <libaegis/change.h>
#include <libaegis/change/file.h>
#include <libaegis/common.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/change/files.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>


//
// NAME
//	sub_change_files - the change_files substitution
//
// SYNOPSIS
//	string_ty *sub_change_files(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_change_files function implements the
//	change_files substitution.  The change_files
//	substitution is used to insert the absolute path of the
//	change_files.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_change_files(sub_context_ty *scp, wstring_list_ty *arg)
{
    change_ty       *cp;
    unsigned        action_mask = 0;
    unsigned        usage_mask = 0;
    bool            not_var = false;
    bool            quote = false;
    string_ty       *s;
    wstring_ty	    *result;

    trace(("sub_change_files()\n{\n"));
    cp = sub_context_change_get(scp);
    if (!cp)
    {
	sub_context_error_set(scp, i18n("not valid in current context"));
	trace(("}\n"));
	return 0;
    }

    //
    // See if we recognize any of these words.
    //
    for (size_t n = 1; n < arg->size(); ++n)
    {
	bool ok = false;
	s = wstr_to_str(arg->get(n));
	for (unsigned k = 0; k < file_action_max; ++k)
	{
	    if (0 == strcmp(s->str_text, file_action_ename((file_action_ty)k)))
	    {
		action_mask |= 1 << k;
		ok = true;
	    }
	}
	for (size_t j = 0; j < file_usage_max; ++j)
	{
	    if (0 == strcmp(s->str_text, file_usage_ename((file_usage_ty)j)))
	    {
		usage_mask |= 1 << j;
		ok = true;
	    }
	}
	if (0 == strcmp(s->str_text, "not") || 0 == strcmp(s->str_text, "!"))
	{
	    not_var = !not_var;
	    ok = true;
	}
	if (0 == strcmp(s->str_text, "quote"))
	{
	    quote = true;
	    ok = true;
	}
	if (!ok)
	{
	    sub_context_error_set(scp, i18n("file qualifier unknown"));
	    trace(("}\n"));
	    return 0;
	}
	str_free(s);
    }

    //
    // Cope with some boundary conditions.
    //
    if (not_var)
    {
	action_mask = ~action_mask;
	usage_mask = ~usage_mask;
    }
    if (action_mask == 0)
	action_mask = ~0;
    if (usage_mask == 0)
	usage_mask = ~0;

    //
    // Look for files matching what they asked for.
    //
    string_list_ty sl;
    for (size_t m = 0; ; ++m)
    {
	fstate_src_ty *src = change_file_nth(cp, m, view_path_first);
	if (!src)
	    break;
	if
	(
	    (action_mask & (1 << src->action))
	&&
	    (usage_mask & (1 << src->usage))
	)
	{
	    if (quote)
	    {
		s = str_quote_shell(src->file_name);
		sl.push_back(s);
		str_free(s);
	    }
	    else
		sl.push_back(src->file_name);
	}
    }

    //
    // Turn it into a space-separated string.
    //
    s = sl.unsplit();
    result = str_to_wstr(s);
    str_free(s);
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
