//
// aegis - project change supervisor
// Copyright (C) 2010 Peter Miller
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

#include <common/nstring/accumulator.h>
#include <common/nstring/list.h>


nstring
nstring_list::unsplit(size_t start, size_t length, const char *separator)
    const
{
    if (separator && !*separator)
        separator = 0;
    size_t end = start + length;
    if (end > content.size())
        end = content.size();
    nstring_accumulator ac;
    for (size_t j = start; j < end; ++j)
    {
        if (separator && j != start)
            ac.push_back(separator);
        string_ty *s = content[j];
        ac.push_back(s->str_text, s->str_length);
    }
    return ac.mkstr();
}
