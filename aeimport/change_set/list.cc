//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004 Peter Miller;
//      All rights reserved.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate lists
//

#include <ac/stdlib.h>

#include <change_set/list.h>
#include <mem.h>


change_set_list_ty *
change_set_list_new(void)
{
    change_set_list_ty *cslp;

    cslp = (change_set_list_ty *)mem_alloc(sizeof(change_set_list_ty));
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
            change_set_delete(cslp->item[j]);
    if (cslp->item)
            mem_free(cslp->item);
    cslp->length = 0;
    cslp->maximum = 0;
    cslp->item = 0;
    mem_free(cslp);
}


#ifdef DEBUG

void
change_set_list_validate(change_set_list_ty *cslp)
{
    size_t          j;

    for (j = 0; j < cslp->length; ++j)
        change_set_validate(cslp->item[j]);
}

#endif


void
change_set_list_append(change_set_list_ty *cslp, change_set_ty *csp)
{
    size_t          nbytes;

    if (cslp->length >= cslp->maximum)
    {
        cslp->maximum = cslp->maximum * 2 + 4;
        nbytes = cslp->maximum * sizeof(cslp->item[0]);
        cslp->item = (change_set_ty **)mem_change_size(cslp->item, nbytes);
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
