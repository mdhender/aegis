//
// aegis - project change supervisor
// Copyright (C) 1999, 2001, 2003-2006, 2008, 2011, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <libaegis/os/isa/path_prefix.h>
#include <common/str.h>


bool
os_isa_path_prefix(const nstring &s1, const nstring &s2)
{
    if (s1.empty())
        return true;
    if (s1.size() == 1 && s1[0] == '.' && s2[0] != '/')
        return true;
    return
        (
            s1.size() < s2.size()
        &&
            !memcmp(s1.c_str(), s2.c_str(), s1.size())
        &&
            s2[s1.size()] == '/'
        );
}


bool
os_isa_path_prefix(string_ty *s1, string_ty *s2)
{
    if (!s1->str_length)
        return true;
    if
    (
        s1->str_length == 1
    &&
        s1->str_text[0] == '.'
    &&
        s2->str_text[0] != '/'
    )
        return true;
    return
        (
            s1->str_length < s2->str_length
        &&
            !memcmp(s1->str_text, s2->str_text, s1->str_length)
        &&
            s2->str_text[s1->str_length] == '/'
        );
}


// vim: set ts=8 sw=4 et :
