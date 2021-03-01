//
//      aegis - project change supervisor
//      Copyright (C) 2004-2009, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>


string_ty *
change_brief_description_get(change::pointer cp)
{
    cstate_ty       *cstate_data;

    cstate_data = cp->cstate_get();
    return cstate_data->brief_description;
}


nstring
change::brief_description_get(void)
{
    cstate_ty *csd = cstate_get();
    return nstring(csd->brief_description);
}


nstring
change::description_get(void)
{
    cstate_ty *csd = cstate_get();
    return nstring(csd->description);
}


// vim: set ts=8 sw=4 et :
