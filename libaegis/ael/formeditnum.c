/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate formeditnums
 */

#include <ael/formeditnum.h>
#include <error.h> /* for assert */
#include <output.h>


void
list_format_edit_number(output_ty *edit_col, fstate_src_ty *src_data)
{
    if (src_data->edit_origin && src_data->edit)
    {
	/*
	 * We have both the original version copied, and the
	 * current head revision.  Print them both, with a
	 * notation implying ``from the old one to the new one''
	 * if they differ.  Only print one if thay are the same.
	 */
	assert(src_data->edit->revision);
	assert(src_data->edit_origin->revision);
	output_fprintf
	(
	    edit_col,
	    "%4s",
	    src_data->edit_origin->revision->str_text
	);
	if
	(
	    str_equal
	    (
		src_data->edit->revision,
		src_data->edit_origin->revision
	    )
	)
	    return;
	output_fprintf(edit_col, " -> %s", src_data->edit->revision->str_text);
	return;
    }

    if (src_data->edit_origin)
    {
	/*
	 * The "original version" copied.
	 */
	assert(src_data->edit_origin->revision);
	output_fprintf
	(
	    edit_col,
	    "%4s",
	    src_data->edit_origin->revision->str_text
	);
    }
    if (src_data->edit)
    {
	/*
	 * For active branches, the current
	 * head revision.  For completed changes
	 * and branches, the revision at aeipass.
	 */
	assert(src_data->edit->revision);
	output_fprintf(edit_col, "%4s", src_data->edit->revision->str_text);
    }
}
