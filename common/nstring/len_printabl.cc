//
// aegis - project change supervisor
// Copyright (C) 2008 Walter Franzini
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/ctype.h>
#include <common/ac/limits.h>

#include <common/nstring.h>


int
nstring::len_printable(int len_max)
    const
{
    if (!ref)
        return 0;
    assert(len_max > 0);
    assert(len_max < INT_MAX);
    if (size_t(len_max) > ref->str_length)
        len_max = int(ref->str_length);
    const char *cp = ref->str_text;
    const char *end = cp + len_max;
    // Intentionally the C locale, not the user's locale
    for (; cp < end && isprint((unsigned char)*cp); ++cp)
        ;
    return (cp - ref->str_text);
}


// vim: set ts=8 sw=4 et :
