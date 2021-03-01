//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#include <common/ac/stdlib.h>

#include <aeimport/format/version_list.h>


format_version_list_ty *
format_version_list_new(void)
{
    format_version_list_ty *fvlp;

    fvlp = new format_version_list_ty;
    fvlp->item = 0;
    fvlp->length = 0;
    fvlp->maximum = 0;
    return fvlp;
}


void
format_version_list_delete(format_version_list_ty *fvlp, int delmore)
{
    size_t          j;

    if (delmore)
    {
        for (j = 0; j < fvlp->length; ++j)
            delete(fvlp->item[j]);
    }
    delete [] fvlp->item;
    fvlp->item = 0;
    fvlp->length = 0;
    fvlp->maximum = 0;
    delete fvlp;
}


#ifdef DEBUG

void
format_version_list_validate(format_version_list_ty *fvlp)
{
    size_t          j;

    for (j = 0; j < fvlp->length; ++j)
        fvlp->item[j]->validate();
}

#endif


void
format_version_list_append(format_version_list_ty *fvlp, format_version_ty *fvp)
{
    if (fvlp->length >= fvlp->maximum)
    {
        fvlp->maximum = fvlp->maximum * 2 + 8;
	format_version_ty **new_item = new format_version_ty * [fvlp->maximum];
	for (size_t j = 0; j < fvlp->length; ++j)
	    new_item[j] = fvlp->item[j];
	delete [] fvlp->item;
	fvlp->item = new_item;
    }
    fvlp->item[fvlp->length++] = fvp;
}


static int
cmp(const void *va, const void *vb)
{
    format_version_ty *a;
    format_version_ty *b;

    a = *(format_version_ty **)va;
    b = *(format_version_ty **)vb;
    if (a->when < b->when)
        return -1;
    if (a->when > b->when)
        return 1;
    return 0;
}


void
format_version_list_sort_by_date(format_version_list_ty *fvlp)
{
    qsort(fvlp->item, fvlp->length, sizeof(fvlp->item[0]), cmp);
}
