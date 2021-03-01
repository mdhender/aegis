//
// aegis - project change supervisor
// Copyright (C) 2001, 2002, 2004-2006, 2008, 2012 Peter Miller
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

#include <common/ac/stdlib.h>

#include <common/debug.h>

#include <aeimport/change_set/list.h>


change_set_list_ty *
change_set_list_new(void)
{
    change_set_list_ty *cslp;

    cslp = new change_set_list_ty;
    cslp->length = 0;
    cslp->maximum = 0;
    cslp->item = 0;
    return cslp;
}


void
change_set_list_delete(change_set_list_ty *cslp)
{
    size_t          j;

    for (j = 0; j < cslp->length; ++j)
        delete cslp->item[j];
    delete [] cslp->item;
    cslp->length = 0;
    cslp->maximum = 0;
    cslp->item = 0;
    delete cslp;
}


#ifdef DEBUG

void
change_set_list_validate(change_set_list_ty *cslp)
{
    size_t          j;

    for (j = 0; j < cslp->length; ++j)
        cslp->item[j]->validate();
}

#endif


void
change_set_list_append(change_set_list_ty *cslp, change_set_ty *csp)
{
    if (cslp->length >= cslp->maximum)
    {
        cslp->maximum = cslp->maximum * 2 + 4;
        change_set_ty **new_item = new change_set_ty * [cslp->maximum];
        for (size_t j = 0; j < cslp->length; ++j)
            new_item[j] = cslp->item[j];
        delete [] cslp->item;
        cslp->item = new_item;
    }
    cslp->item[cslp->length++] = csp;
}


static int
cmp(const void *va, const void *vb)
{
    change_set_ty   *a;
    change_set_ty   *b;

    a = *(change_set_ty **)va;
    b = *(change_set_ty **)vb;
    if (a->when < b->when)
        return -1;
    if (a->when > b->when)
        return 1;
    return 0;
}


void
change_set_list_sort_by_date(change_set_list_ty *cslp)
{
    qsort(cslp->item, cslp->length, sizeof(cslp->item[0]), cmp);
}


// vim: set ts=8 sw=4 et :
