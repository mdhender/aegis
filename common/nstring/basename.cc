//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/nstring.h>


nstring
nstring::basename(const nstring &suffix)
    const
{
    const char *ep = c_str() + size();
    const char *cp = strrchr(c_str(), '/');
    if (cp)
        ++cp;
    else
        cp = c_str();
    size_t nbytes = ep - cp;
    if (!suffix.empty())
    {
        if
        (
            nbytes >= suffix.size()
        &&
            0 == memcmp(ep - suffix.size(), suffix.c_str(), suffix.size())
        )
        {
            nbytes -= suffix.size();
        }
    }
    return nstring(cp, nbytes);
}
