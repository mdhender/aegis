//
//      aegis - project change supervisor
//      Copyright (C) 2005, 2006, 2008, 2009, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/nstring.h>
#include <common/nstring/accumulator.h>


nstring
nstring::replace(const nstring &lhs, const nstring &rhs, int maximum)
    const
{
    static nstring_accumulator sa;
    sa.clear();

    //
    // Deal with some trivial cases.
    //
    if (lhs.size() == 0 || maximum == 0)
        return *this;

    //
    // Default the number of times, if necessary.
    //
    if (maximum < 0)
        maximum = size() + 1;

    //
    // Walk along the string replacing things.
    //
    const char *ip = c_str();
    const char *ip_end = ip + size();
    while (ip < ip_end && (size_t)(ip_end - ip) >= lhs.size())
    {
        const void *p = memmem(ip, ip_end - ip, lhs.c_str(), lhs.size());
        if (!p)
            break;
        const char *pp = (const char *)p;
        sa.push_back(ip, pp - ip);
        ip = pp;
        sa.push_back(rhs);
        ip += (lhs.empty() ? 1 : lhs.size());
        if (--maximum <= 0)
            break;
    }

    //
    // Collect the tail-end of the input.
    //
    if (ip < ip_end)
        sa.push_back(ip, (size_t)(ip_end - ip));

    //
    // Build ther answer.
    //
    return sa.mkstr();
}


// vim: set ts=8 sw=4 et :
