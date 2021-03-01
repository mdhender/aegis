//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini;
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
// MANIFEST: implementation of the change_functor_invent_build class
//

#pragma implementation "change_functor_invent_build"

#include <ac/string.h>

#include <change/functor/invent_build.h>
#include <uuidentifier.h>

change_functor_inventory_builder::~change_functor_inventory_builder()
{
    if (pp)
        project_free(pp);
    stp = 0;
}


change_functor_inventory_builder::change_functor_inventory_builder
(
    project_ty *proj,
    symtab<change_ty> *symt
) :
    change_functor()
{
    stp = symt;                 // fixme: need proper copy?
    pp = project_copy(proj);
}


void
change_functor_inventory_builder::operator()(change_ty *cp)
{
    cstate_ty *cstate_data = change_cstate_get(cp);
    if (cstate_data->uuid)
        stp->assign(cstate_data->uuid, cp);

    if (cstate_data->attribute && cstate_data->attribute->length)
    {
	for (size_t j = 0; j < cstate_data->attribute->length; ++j)
	{
	    attributes_ty *ap = cstate_data->attribute->list[j];
	    if
	    (
		ap->name
	    &&
		0 == strcasecmp(ap->name->str_text, ORIGINAL_UUID)
	    &&
		ap->value
	    &&
		// users can edit, we will check
		universal_unique_identifier_valid(ap->value)
	    )
	    {
                stp->assign(ap->value, cp);
	    }
	}
    }
}
