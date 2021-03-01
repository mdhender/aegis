//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2003-2008 Peter Miller
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
#include <common/error.h> // for assert


void
change_architecture_from_pconf(change::pointer cp)
{
    pconf_ty        *pconf_data;
    size_t          j;

    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    change_architecture_clear(cp);
    for (j = 0; j < pconf_data->architecture->length; ++j)
    {
	pconf_architecture_ty *ap;

	ap = pconf_data->architecture->list[j];
	if (ap->mode == pconf_architecture_mode_required)
	{
	    //
	    // We only transfer the architecture names marked
	    // "required", but this is a project thing.
	    // Once the architecture names are listed
	    // against the change, they become mandatory
	    // for the change.
	    //
	    change_architecture_add(cp, ap->name);
	}
    }
}
