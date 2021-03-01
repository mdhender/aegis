//
//	aegis - project change supervisor
//	Copyright (C) 1999-2002, 2004-2008 Peter Miller
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

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <common/error.h> // for assert
#include <common/trace.h>


string_ty *
change_development_directory_get(change::pointer cp, int resolve)
{
    string_ty	*result;

    trace(("change_development_directory_get(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    if (!resolve)
    {
	if (!cp->development_directory_unresolved)
	{
	    result = change_top_path_get(cp, 0);
	    if (change_was_a_branch(cp))
		result = str_format("%s/baseline",  result->str_text);
	    else
		result = str_copy(result);
	    cp->development_directory_unresolved = result;
	}
	result = cp->development_directory_unresolved;
    }
    else
    {
	if (!cp->development_directory_resolved)
	{
	    result = change_top_path_get(cp, 1);
	    if (change_was_a_branch(cp))
		result = str_format("%s/baseline", result->str_text);
	    else
		result = str_copy(result);
	    cp->development_directory_resolved = result;
	}
	result = cp->development_directory_resolved;
    }

    trace(("result = \"%s\"\n", result->str_text));
    trace(("}\n"));
    return result;
}
