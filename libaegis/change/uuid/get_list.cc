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
#include <common/uuidentifier.h>

#include <common/error.h> // for assert
#include <common/nstring/list.h>
#include <libaegis/change.h>


void
change::uuid_get_list(nstring_list &result)
{
    cstate_ty *csp = cstate_get();
    if (csp->uuid)
        result.push_back(nstring(csp->uuid));
    attributes_list_ty *alp = csp->attribute;
    if (!alp)
        return;
    for (size_t j = 0; j < alp->length; ++j)
    {
        attributes_ty *ap = alp->list[j];
        assert(ap);
        assert(ap->name);
        if (!ap->name)
            continue;
        assert(ap->value);
        if (!ap->value)
            continue;
        if (0 == strcasecmp(ap->name->str_text, ORIGINAL_UUID))
            result.push_back(nstring(ap->value));
    }
}
