//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
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

#include <common/ac/stdlib.h>

#include <common/nstring/list.h>
#include <common/sizeof.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/project/history.h>


static int
icmp(const void *va, const void *vb)
{
    const int *a = (const int *)va;
    const int *b = (const int *)vb;
    if (*a == *b)
        return 0;
    if (*a < *b)
        return -1;
    return 1;
}


nstring
change::copyright_years_get_string()
{
    //
    // Extract the copyright years from the project
    // and the change.  Don't worry about duplicates.
    //
    int ary[100]; // a century should be enough  :-)
    int ary_len;
    project_copyright_years_get(this->pp, ary, SIZEOF(ary), &ary_len);
    int ary_len2;
    change_copyright_years_get
    (
        this,
        ary + ary_len,
        (size_t)(SIZEOF(ary) - ary_len),
        &ary_len2
    );
    ary_len += ary_len2;

    //
    // sort the array
    //
    qsort(ary, (size_t)ary_len, sizeof(ary[0]), icmp);

    //
    // Build the text string for the result.
    // This is where duplicates are removed.
    //
    nstring_list wl;
    for (int j = 0; j < ary_len; ++j)
    {
        if (j && ary[j - 1] == ary[j])
            continue;
        nstring s = nstring::format("%d", ary[j]);
        wl.push_back(s);
    }
    return wl.unsplit(", ");
}


// vim: set ts=8 sw=4 et :
