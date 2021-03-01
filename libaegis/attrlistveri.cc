//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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
#include <libaegis/common.fmtgen.h>
#include <libaegis/sub.h>


void
attributes_list_verify(attributes_list_ty *d)
{
    size_t          j;

    if (!d)
        return;
    for (j = 0; j < d->length; ++j)
    {
        attributes_ty   *ap;

        ap = d->list[j];
        if (!ap->name)
        {
            sub_context_ty      *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", ap->errpos);
            sub_var_set_charstar(scp, "FieLD_Name", "name");
            fatal_intl
            (
                scp,
                i18n("$filename: contains no \"$field_name\" field")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
        if (!ap->value)
        {
            sub_context_ty      *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "File_Name", ap->errpos);
            sub_var_set_charstar(scp, "FieLD_Name", "value");
            fatal_intl
            (
                scp,
                i18n("$filename: contains no \"$field_name\" field")
            );
            // NOTREACHED
            sub_context_delete(scp);
        }
    }
}


// vim: set ts=8 sw=4 et :
