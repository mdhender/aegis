//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/string.h>
#include <common/ac/time.h>

#include <common/debug.h>
#include <common/yyyymmdd_wk.h>


int
yyyymmdd_to_wday(int yyyy, int mm, int dd)
{
    struct tm tim;
    memset(&tim, 0, sizeof(tim));
    tim.tm_mday = dd;
    tim.tm_mon = mm - 1;
    tim.tm_year = yyyy - 1900;
#ifdef DEBUG
    time_t when = mktime(&tim);
    assert(when != (time_t)(-1));
#else
    mktime(&tim);
#endif
    return tim.tm_wday;
}


int
days_in_month(int year, int month)
{
    switch (month)
    {
    case 2:
        if (year % 4 == 0)
        {
            if (year % 400 != 0)
                return 29;
            if (year % 2000 != 0)
                return 29;
        }
        return 28;

    case 4:
    case 6:
    case 9:
    case 11:
        return 30;

    default:
        return 31;
    }
}


// vim: set ts=8 sw=4 et :
