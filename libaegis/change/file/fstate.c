/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001-2003 Peter Miller;
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
#include <sub.h>
#include <symtab.h>
#include <trace.h>


static void
fimprove(fstate_ty *fstate_data, string_ty *filename, change_ty *cp)
{
    size_t	    j;

    if (!fstate_data->src)
	fstate_data->src = fstate_src_list_type.alloc();

    /*
     * Migrate file state information.
     */
    for (j = 0; j < fstate_data->src->length; ++j)
    {
	fstate_src_ty   *src;

	src = fstate_data->src->list[j];

	/*
	 * Fix an ugly inconsistency in the file action.
	 */
	if (src->deleted_by)
	    src->action = file_action_remove;

	/*
	 * Historical 4.9 -> 4.10 transition.
	 *
	 * This is here to cope with cases where the users upgrade with
	 * changes between 'being reviewed' and 'being integrated' states.
	 *
	 * This code must agree with the corresponding code in
	 * libaegis/project.c
	 */
	switch (src->action)
	{
	case file_action_transparent:
	case file_action_remove:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    if (src->about_to_be_created_by || src->about_to_be_copied_by)
		src->action = file_action_transparent;
	    break;
	}

	/*
	 * Historical 2.3 -> 3.0 transition.
	 *
	 * This covers a transitional glitch in the edit number
	 * semantics.  Very few installed sites will ever
	 * need this.
	 */
	if (src->edit_number && !src->edit_number_origin)
	    src->edit_number_origin = str_copy(src->edit_number);

	/*
	 * Historical 3.24 to 3.25 transition.
	 *
	 * This was when history file contents encoding was added,
	 * so that we could cope with binary files transparently,
	 * even for ascii-only history tools.
	 */
	if (src->edit_number)
	{
	    src->edit = history_version_type.alloc();
	    src->edit->revision = src->edit_number;
	    src->edit->encoding = history_version_encoding_none;
	    src->edit_number = 0;
	}
	if (src->edit_number_origin)
	{
	    src->edit_origin = history_version_type.alloc();
	    src->edit_origin->revision = src->edit_number_origin;
	    src->edit_origin->encoding = history_version_encoding_none;
	    src->edit_number_origin = 0;
	}
	if (src->edit_number_origin_new)
	{
	    src->edit_origin_new = history_version_type.alloc();
	    src->edit_origin_new->revision = src->edit_number_origin_new;
	    src->edit_origin_new->encoding = history_version_encoding_none;
	    src->edit_number_origin_new = 0;
	}

	/*
	 * Make sure things are where they are meant to be.
	 */
	if (src->edit && !src->edit->revision)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    sub_var_set_charstar(scp, "FieLD_Name", "edit.revision");
	    change_fatal
	    (
		cp,
		scp,
		i18n("$filename: contains no \"$field_name\" field")
	    );
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	if (src->edit_origin && !src->edit_origin->revision)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    sub_var_set_charstar(scp, "FieLD_Name", "edit_origin.revision");
	    change_fatal
	    (
		cp,
		scp,
		i18n("$filename: contains no \"$field_name\" field")
	    );
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	if (src->edit_origin_new && !src->edit_origin_new->revision)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    sub_var_set_charstar(scp, "FieLD_Name", "edit_origin_new.revision");
	    change_fatal
	    (
		cp,
		scp,
		i18n("$filename: contains no \"$field_name\" field")
	    );
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
    }
}


fstate_ty *
change_fstate_get(change_ty *cp)
{
    cstate_ty       *cstate_data;
    string_ty	    *fn;
    size_t	    j;

    /*
     * make sure the change state has been read in,
     * in case its src field needed to be converted.
     * (also to ensure lock_sync has been called for both)
     */
    trace(("change_fstate_get(cp = %08lX)\n{\n", (long)cp));
    cstate_data = change_cstate_get(cp);

    if (!cp->fstate_data)
    {
	fn = change_fstate_filename_get(cp);
	change_become(cp);
	cp->fstate_data = fstate_read_file(fn);
	change_become_undo();
	fimprove(cp->fstate_data, fn, cp);
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
	    fstate_src_ty   *p;

	    p = cp->fstate_data->src->list[j];
	    symtab_assign(cp->fstate_stp, p->file_name, p);
	}
    }
    trace(("return %08lX;\n", (long)cp->fstate_data));
    trace(("}\n"));
    return cp->fstate_data;
}
