//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate clears
//

#include <change.h>
#include <change/attributes.h>
#include <error.h>              // for assert
#include <str.h>
#include <uuidentifier.h>

void
change_uuid_clear(change_ty *cp)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    if (cstate_data->uuid)
    {
        assert(universal_unique_identifier_valid(cstate_data->uuid));
        change_attributes_append
	(
	    cstate_data,
	    ORIGINAL_UUID,
	    cstate_data->uuid->str_text
	);
        str_free(cstate_data->uuid);
        cstate_data->uuid = 0;
    }
}
