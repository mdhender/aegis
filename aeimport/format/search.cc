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
// MANIFEST: functions to manipulate searchs
//

#include <error.h>
#include <format/search.h>
#include <mem.h>
#include <str_list.h>


format_search_ty *
format_search_new(void)
{
    format_search_ty *fsp =
        (format_search_ty *)mem_alloc(sizeof(format_search_ty));
    fsp->filename_physical = 0;
    fsp->filename_logical = 0;
    fsp->root = 0;
    return fsp;
}


void
format_search_delete(format_search_ty *fsp)
{
    if (fsp->filename_physical)
    {
        str_free(fsp->filename_physical);
        fsp->filename_physical = 0;
    }
    if (fsp->filename_logical)
    {
        str_free(fsp->filename_logical);
        fsp->filename_logical = 0;
    }
    if (fsp->root)
    {
        format_version_delete(fsp->root);
        fsp->root = 0;
    }
    mem_free(fsp);
}


void
format_search_staff(format_search_ty *fsp, string_list_ty *slp)
{
    format_version_ty *fvp;

    for (fvp = fsp->root; fvp; fvp = fvp->after)
    {
        // what about branches?
        assert(fvp->who);
        string_list_append_unique(slp, fvp->who);
    }
}


#ifdef DEBUG

void
format_search_validate(format_search_ty *fsp)
{
    assert(str_validate(fsp->filename_physical));
    assert(str_validate(fsp->filename_logical));
    format_version_validate(fsp->root);
}

#endif
