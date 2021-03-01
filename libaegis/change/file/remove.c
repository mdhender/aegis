/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate removes
 */

#include <change/file.h>
#include <error.h>
#include <symtab.h>
#include <trace.h>


void
change_file_remove(change_ty *cp, string_ty *file_name)
{
    fstate_ty       *fstate_data;
    int             j;
    fstate_src_ty   *src_data;

    trace(("change_file_remove(cp = %08lX, file_name = \"%s\")\n{\n",
	(long)cp, file_name->str_text));
    fstate_data = change_fstate_get(cp);
    assert(fstate_data->src);
    assert(cp->fstate_stp);
    symtab_delete(cp->fstate_stp, file_name);
    for (j = 0; j < fstate_data->src->length; ++j)
    {
	src_data = fstate_data->src->list[j];
	if (!str_equal(src_data->file_name, file_name))
	    continue;
	fstate_src_type.free(src_data);
	fstate_data->src->list[j] =
	    fstate_data->src->list[--fstate_data->src->length];
	break;
    }
    trace(("}\n"));
}
