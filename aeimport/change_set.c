/*
 *      aegis - project change supervisor
 *      Copyright (C) 2001, 2002 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate change_sets
 */

#include <change_set.h>
#include <mem.h>


change_set_ty *
change_set_new(void)
{
    change_set_ty   *csp;

    csp = (change_set_ty *)mem_alloc(sizeof(change_set_ty));
    csp->who = 0;
    csp->when = 0;
    csp->description = 0;
    change_set_file_list_constructor(&csp->file);
    string_list_constructor(&csp->tag);
    return csp;
}


void
change_set_delete(change_set_ty *csp)
{
    if (csp->who)
    {
        str_free(csp->who);
        csp->who = 0;
    }
    csp->when = 0;
    if (csp->description)
    {
        str_free(csp->description);
        csp->description = 0;
    }
    change_set_file_list_destructor(&csp->file);
    string_list_destructor(&csp->tag);
    mem_free(csp);
}


#ifdef DEBUG

void
change_set_validate(change_set_ty *csp)
{
    if (csp->who)
        str_validate(csp->who);
    if (csp->description)
        str_validate(csp->description);
    change_set_file_list_validate(&csp->file);
    string_list_validate(&csp->tag);
}

#endif
