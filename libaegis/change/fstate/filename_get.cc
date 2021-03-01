//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2004-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//
// MANIFEST: functions to manipulate filename_gets
//

#include <libaegis/change.h>
#include <common/error.h> // for assert


string_ty *
change_fstate_filename_get(change::pointer cp)
{
    assert(cp->reference_count >= 1);
    if (!cp->fstate_filename)
    {
	cp->fstate_filename =
    	    str_format("%s.fs", change_cstate_filename_get(cp)->str_text);
    }
    return cp->fstate_filename;
}


string_ty *
change_pfstate_filename_get(change::pointer cp)
{
    assert(cp->reference_count >= 1);
    if (!cp->pfstate_filename)
    {
	cp->pfstate_filename =
    	    str_format("%s.pfs", change_cstate_filename_get(cp)->str_text);
    }
    return cp->pfstate_filename;
}
