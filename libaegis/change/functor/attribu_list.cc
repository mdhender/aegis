//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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
#include <common/uuidentifier.h>
#include <libaegis/attribute.h>
#include <libaegis/change/functor/attribu_list.h>
#include <libaegis/cstate.h>


change_functor_attribute_list::~change_functor_attribute_list()
{
    alp = 0;
}


change_functor_attribute_list::change_functor_attribute_list(
	bool arg1, attributes_list_ty *arg2) :
    change_functor(arg1),
    alp(arg2)
{
    assert(alp);
}


void
change_functor_attribute_list::remember(string_ty *s)
{
    attributes_list_append_unique(alp, ORIGINAL_UUID, s);
}


void
change_functor_attribute_list::operator()(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->uuid)
	remember(cstate_data->uuid);
    if (cstate_data->uuid)
    {
	assert(universal_unique_identifier_valid(cstate_data->uuid));
	remember(cstate_data->uuid);
    }
    if (!cstate_data->attribute)
	return;
    for (size_t j = 0; j < cstate_data->attribute->length; ++j)
    {
	attributes_ty *ap = cstate_data->attribute->list[j];
	assert(ap->name);
	assert(ap->value);
	if
	(
	    ap->name
	&&
	    0 == strcasecmp(ORIGINAL_UUID, ap->name->str_text)
	&&
	    ap->value
	&&
	    // users can edit, we will check
	    universal_unique_identifier_valid(ap->value)
	)
	{
	    remember(ap->value);
	}
    }
}
