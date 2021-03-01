//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
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

#include <common/error.h> // for assert
#include <libaegis/change.h>

#include <aede-policy/validation.h>


bool
validation::was_downloaded(change::pointer cp)
{
    cstate_history_list_ty *hlp = cp->cstate_get()->history;
    assert(hlp);
    if (!hlp)
	return false;
    for (size_t j = 0; j < hlp->length; ++j)
    {
	cstate_history_ty *hp = hlp->list[j];
	if
       	(
	    hp->what == cstate_history_what_develop_begin
	&&
	    (hp->why && 0 == memcmp(hp->why->str_text, "Downloaded", 10))
	)
	    return true;
    }
    return false;
}
