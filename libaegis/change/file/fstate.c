/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate fstates
 */

#include <change/file.h>
#include <symtab.h>
#include <trace.h>


static void fimprove _((fstate));

static void
fimprove(fstate_data)
	fstate		fstate_data;
{
	size_t		j;

	if (!fstate_data->src)
		fstate_data->src = fstate_src_list_type.alloc();

	/*
	 * This covers a transitional glitch in the edit number
	 * semantics.  Very few installed sites will ever need this.
	 */
	for (j = 0; j < fstate_data->src->length; ++j)
	{
		fstate_src	src;

		src = fstate_data->src->list[j];
		/* Historical 2.3 -> 3.0 transition. */
		if (src->edit_number && !src->edit_number_origin)
			src->edit_number_origin = str_copy(src->edit_number);
	}
}


fstate
change_fstate_get(cp)
	change_ty	*cp;
{
	cstate		cstate_data;
	string_ty	*fn;
	size_t		j;

	/*
	 * make sure the change state has been read in,
	 * in case its src field needed to be converted.
	 * (also to ensure lock_sync has been called for both)
	 */
	trace(("change_fstate_get(cp = %08lX)\n{\n"/*}*/, cp));
	cstate_data = change_cstate_get(cp);

	if (!cp->fstate_data)
	{
		fn = change_fstate_filename_get(cp);
		change_become(cp);
		cp->fstate_data = fstate_read_file(fn->str_text);
		change_become_undo();
		fimprove(cp->fstate_data);
	}
	if (!cp->fstate_data->src)
		cp->fstate_data->src = fstate_src_list_type.alloc();

	/*
	 * Create an O(1) index.
	 * This speeds up just about everything.
	 */
	if (!cp->fstate_stp)
	{
		cp->fstate_stp = symtab_alloc(cp->fstate_data->src->length);
		for (j = 0; j < cp->fstate_data->src->length; ++j)
		{
			fstate_src	p;

			p = cp->fstate_data->src->list[j];
			symtab_assign(cp->fstate_stp, p->file_name, p);
		}
	}
	trace(("return %08lX;\n", cp->fstate_data));
	trace((/*{*/"}\n"));
	return cp->fstate_data;
}
