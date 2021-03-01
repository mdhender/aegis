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

#include <common/ac/stdio.h>

#include <common/format_elpsd.h>
#include <common/nstring.h>


void
format_elapsed(char *buf, size_t len, time_t secs)
{
    time_t mins = secs / 60;
    secs %= 60;
    time_t hours = mins / 60;
    mins %= 60;
    if (hours == 0)
    {
	snprintf(buf, len, "%2dm%2.2ds", (int)mins, (int)secs);
	return;
    }
    time_t days = hours / 24;
    hours %= 24;
    if (days == 0)
    {
	snprintf(buf, len, "%2dh%2.2dm", (int)hours, (int)mins);
	return;
    }
    if (days < 100)
    {
	snprintf(buf, len, "%2dd%2.2dh", (int)days, (int)hours);
	return;
    }
    snprintf(buf, len, "%5.2fy", days / 365.25);
}


nstring
format_elapsed(time_t nsec)
{
    char buffer[100];
    format_elapsed(buffer, sizeof(buffer), nsec);
    return nstring(buffer);
}
