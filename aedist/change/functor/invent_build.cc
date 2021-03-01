//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Walter Franzini
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

#include <common/error.h>              // for assert
#include <aedist/change/functor/invent_build.h>
#include <common/uuidentifier.h>
#include <common/trace.h>

change_functor_inventory_builder::~change_functor_inventory_builder()
{
    if (pp)
        project_free(pp);
    stp = 0;
}


change_functor_inventory_builder::change_functor_inventory_builder(bool arg1,
	bool arg2, bool arg3, project_ty *arg4, symtab<change> *arg5) :
    change_functor(arg1, arg2),
    inou(arg3),
    stp(arg5),
    pp(project_copy(arg4))
{
}


void
change_functor_inventory_builder::operator()(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();
    assert(cstate_data);
    time_t cp_ipass_when =
        change_when_get(cp, cstate_history_what_integrate_pass);
    trace_time(cp_ipass_when);
    if (cstate_data->uuid)
    {
        //
        // We do not override a UUID which is already present.  It comes
        // from the original-UUID attribute so it is more recent than
        // the change the UUID belongs to.  Thus is should be a better
        // merge point to select, it should generate smaller (logical)
        // conflicts.
        //
        change::pointer cp2 = stp->query(cstate_data->uuid);
        if (cp2)
        {
            time_t cp2_ipass_when =
                change_when_get(cp2, cstate_history_what_integrate_pass);
            trace_time(cp2_ipass_when);
            if (cp_ipass_when > cp2_ipass_when)
            {
                stp->assign(cstate_data->uuid, change_copy(cp));
                change_free(cp2);
            }
        }
        else
            stp->assign(cstate_data->uuid, change_copy(cp));
    }

    if (inou)
        return;

    if (!cstate_data->attribute)
        return;

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
            change::pointer cp2 = stp->query(ap->value);
            if (cp2)
            {
                time_t cp2_ipass_when =
                    change_when_get(cp2, cstate_history_what_integrate_pass);
                trace_time(cp2_ipass_when);
                if (cp_ipass_when > cp2_ipass_when)
                {
                    stp->assign(ap->value, change_copy(cp));
                    change_free(cp2);
                    continue;
                }
            }

            stp->assign(ap->value, change_copy(cp));
        }
    }
}
