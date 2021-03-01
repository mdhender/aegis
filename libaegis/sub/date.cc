//
//	aegis - project change supervisor
//	Copyright (C) 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate dates
//

#include <common/ac/time.h>

#include <common/language.h>
#include <libaegis/sub.h>
#include <libaegis/sub/date.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <common/wstr/list.h>


//
// NAME
//	sub_date - the date substitution
//
// SYNOPSIS
//	wstring_ty *sub_date(wstring_list_ty *arg);
//
// DESCRIPTION
//	The sub_date function implements the date substitution.
//	The date substitution is replaced by the current date.
//	The optional arguments may construct format string,
//	similar to the date(1) command.
//
// ARGUMENTS
//	arg	- list of arguments, including the function name as [0]
//
// RETURNS
//	a pointer to a string in dynamic memory;
//	or NULL on error, setting suberr appropriately.
//

wstring_ty *
sub_date(sub_context_ty *scp, wstring_list_ty *arg)
{
    trace(("sub_date()\n{\n"));

    //
    // We use the time() function directly, rather than the now()
    // function.  This is because the now() function has a static time
    // (for timestamps, etc) by the progress indicators need the correct
    // time.
    //
    time_t when = 0;
    time(&when);

    wstring_ty *result = 0;
    if (arg->size() < 2)
    {
	char *time_string = ctime(&when);
	result = wstr_n_from_c(time_string, 24);
    }
    else
    {
	wstring_ty *wfmt = arg->unsplit(1, arg->size());
	string_ty *fmt = wstr_to_str(wfmt);
	wstr_free(wfmt);
	struct tm *theTm = localtime(&when);

	//
	// The strftime is locale dependent.
	//
	language_human();
	char buf[1000];
	size_t nbytes = strftime(buf, sizeof(buf) - 1, fmt->str_text, theTm);
	language_C();

	if (!nbytes && fmt->str_length)
	{
	    sub_context_error_set(scp, i18n("strftime output too large"));
	    result = 0;
	}
	else
	    result = wstr_n_from_c(buf, nbytes);
	str_free(fmt);
    }
    trace(("return %8.8lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
