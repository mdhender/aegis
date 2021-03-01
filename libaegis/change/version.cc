//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2009, 2012 Peter Miller
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

#include <libaegis/change/branch.h>
#include <libaegis/project.h>
#include <libaegis/zero.h>


nstring
change_version_get(change::pointer cp)
{
    return cp->version_get();
}


nstring
change::version_get(void)
{
    nstring s1(project_version_short_get(pp));
    nstring s2;
    cstate_ty *csd = cstate_get();
    if (s1.length())
    {
        if (csd->state < cstate_state_being_integrated)
        {
            s2 = nstring::format
                (
                    "%s.C%3.3ld",
                    s1.c_str(),
                    magic_zero_decode(number)
                );
        }
        else
        {
            s2 = nstring::format("%s.D%3.3ld", s1.c_str(), csd->delta_number);
        }
    }
    else
    {
        if (csd->state < cstate_state_being_integrated)
            s2 = nstring::format("C%3.3ld", magic_zero_decode(number));
        else
            s2 = nstring::format("D%3.3ld", csd->delta_number);
    }
    return s2;
}


// vim: set ts=8 sw=4 et :
