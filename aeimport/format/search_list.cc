//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate search_lists
//

#include <aeimport/format/search_list.h>
#include <common/mem.h>
#include <common/str_list.h>


format_search_list_ty *
format_search_list_new(void)
{
    format_search_list_ty *fslp;

    fslp = (format_search_list_ty *)mem_alloc(sizeof(format_search_list_ty));
    fslp->item = 0;
    fslp->length = 0;
    fslp->maximum = 0;
    return fslp;
}


void
format_search_list_delete(format_search_list_ty *fslp)
{
    size_t          j;

    for (j = 0; j < fslp->length; ++j)
        format_search_delete(fslp->item[j]);
    if (fslp->item)
        mem_free(fslp->item);
    fslp->item = 0;
    fslp->length = 0;
    fslp->maximum = 0;
    mem_free(fslp);
}


void
format_search_list_append(format_search_list_ty *fslp, format_search_ty *fsp)
{
    if (fslp->length >= fslp->maximum)
    {
        size_t          nbytes;

        fslp->maximum = fslp->maximum * 2 + 8;
        nbytes = fslp->maximum * sizeof(fslp->item[0]);
        fslp->item = (format_search_ty **)mem_change_size(fslp->item, nbytes);
    }
    fslp->item[fslp->length++] = fsp;
}


void
format_search_list_staff(format_search_list_ty *fslp, string_list_ty *slp)
{
    size_t          j;

    for (j = 0; j < fslp->length; ++j)
        format_search_staff(fslp->item[j], slp);
}


#ifdef DEBUG

void
format_search_list_validate(format_search_list_ty *fslp)
{
    size_t          j;

    for (j = 0; j < fslp->length; ++j)
        format_search_validate(fslp->item[j]);
}

#endif


format_search_ty *
format_search_list_query(format_search_list_ty *fslp, string_ty *filename)
{
    size_t          j;

    for (j = 0; j < fslp->length; ++j)
    {
	format_search_ty *fsp;

	fsp = fslp->item[j];
	if (str_equal(fsp->filename_logical, filename))
	    return fsp;
    }
    return 0;
}
