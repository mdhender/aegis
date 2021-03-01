//
// aegis - project change supervisor
// Copyright (C) 2000-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/attrlistveri.h>
#include <libaegis/cattr.fmtgen.h>
#include <libaegis/change/attributes.h>
#include <libaegis/sub.h>


void
change_attributes_verify(string_ty *fn, cattr_ty *d)
{
    if (!d->brief_description)
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", d->errpos ? d->errpos : fn);
        sub_var_set_charstar(scp, "FieLD_Name", "brief_description");
        fatal_intl(scp, i18n("$filename: contains no \"$field_name\" field"));
        // NOTREACHED
        sub_context_delete(scp);
    }
    if (!(d->mask & cattr_cause_mask))
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", d->errpos ? d->errpos : fn);
        sub_var_set_charstar(scp, "FieLD_Name", "cause");
        fatal_intl(scp, i18n("$filename: contains no \"$field_name\" field"));
        // NOTREACHED
        sub_context_delete(scp);
    }
    attributes_list_verify(d->attribute);
}


// vim: set ts=8 sw=4 et :
