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
// MANIFEST: functions to manipulate file_lists
//

#include <change_set/file_list.h>
#include <mem.h>


void
change_set_file_list_constructor(change_set_file_list_ty *csflp)
{
    csflp->item = 0;
    csflp->length = 0;
    csflp->maximum = 0;
}


void
change_set_file_list_destructor(change_set_file_list_ty *csflp)
{
    size_t          j;

    for (j = 0; j < csflp->length; ++j)
        change_set_file_destructor(csflp->item + j);
    if (csflp->item)
        mem_free(csflp->item);
    csflp->item = 0;
    csflp->length = 0;
    csflp->maximum = 0;
}


#ifdef DEBUG

void
change_set_file_list_validate(change_set_file_list_ty *csflp)
{
    size_t          j;

    for (j = 0; j < csflp->length; ++j)
        change_set_file_validate(csflp->item + j);
}

#endif


void
change_set_file_list_append(change_set_file_list_ty *csflp, string_ty *filename,
    string_ty *edit, change_set_file_action_ty action, string_list_ty *tag)
{
    change_set_file_ty *csfp;
    size_t          j;

    //
    // If we already have the file, just update the edit number.
    //
    for (j = 0; j < csflp->length; ++j)
    {
        csfp = csflp->item + j;
        if (str_equal(filename, csfp->filename))
        {
            str_free(csfp->edit);
            csfp->edit = str_copy(edit);
            csfp->action = action;
            return;
        }
    }

    if (csflp->length >= csflp->maximum)
    {
        size_t          nbytes;

        csflp->maximum = csflp->maximum * 2 + 4;
        nbytes = csflp->maximum * sizeof(csflp->item[0]);
        csflp->item =
            (change_set_file_ty *)mem_change_size(csflp->item, nbytes);
    }
    csfp = csflp->item + csflp->length++;
    change_set_file_constructor(csfp, filename, edit, action, tag);
}
