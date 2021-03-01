//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/change/identifi_sub.h>
#include <libaegis/sub.h>


void
change_identifier_subset::error_if_no_explicit_delta(void)
{
    if
    (
        !historian_p
    &&
        delta_name.empty()
    &&
        delta_number == 0
    &&
        delta_date == NO_TIME_SET
    )
    {
        fatal_intl(0, i18n("no delta name"));
    }
}


// vim: set ts=8 sw=4 et :
