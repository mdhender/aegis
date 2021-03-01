//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/change.h>
#include <libaegis/change/cstate/improve.h>
#include <libaegis/change/lock_sync.h>
#include <common/error.h> // for assert
#include <libaegis/sub.h>
#include <common/symtab.h>
#include <common/trace.h>


static void
cstate_to_fstate(change::pointer cp)
{
    trace(("cstate_to_fstate(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    assert(cp->cstate_data);
    assert(cp->cstate_data->src);
    assert(!cp->fstate_data);
    cp->fstate_data = (fstate_ty *)fstate_type.alloc();
    cp->fstate_data->src = (fstate_src_list_ty *)fstate_src_list_type.alloc();
    cp->fstate_is_a_new_file = 1;
    cp->fstate_stp = symtab_alloc(5);
    cp->fstate_uuid_stp = symtab_alloc(5);

    //
    // copy the file records from cstate to fstate
    //
    for (size_t j = 0; j < cp->cstate_data->src->length; ++j)
    {
	cstate_src_ty	*src1;
	fstate_src_ty	*src2;
	fstate_src_ty	**addr_p;
	meta_type		*type_p;

	src1 = cp->cstate_data->src->list[j];
	if
	(
	    !(src1->mask & cstate_src_action_mask)
	||
	    !(src1->mask & cstate_src_usage_mask)
	||
	    !src1->file_name
	||
	    (
		cp->cstate_data->state == cstate_state_completed
	    &&
		!src1->edit_number
	    )
	)
	{
	    sub_context_ty  *scp;

    	    assert(src1->errpos);
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", src1->errpos);
	    sub_var_set_charstar(scp, "FieLD_Name", "src");
	    change_fatal
	    (
		cp,
		scp,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	    sub_context_delete(scp);
	}

	src2 = (fstate_src_ty *)fstate_src_type.alloc();
	src2->errpos = str_copy(src1->errpos);
	addr_p = (fstate_src_ty **)
            fstate_src_list_type.list_parse(cp->fstate_data->src, &type_p);
	assert(type_p == &fstate_src_type);
	*addr_p = src2;

	//
	// copy all of the attributes across
	//
	src2->action = src1->action;
	src2->mask |= fstate_src_action_mask;
	src2->usage = src1->usage;
	src2->mask |= fstate_src_usage_mask;
	src2->file_name = str_copy(src1->file_name);
	if (src1->edit_number)
	{
	    src2->edit = (history_version_ty *)history_version_type.alloc();
	    src2->edit->errpos = str_copy(src1->errpos);
	    src2->edit->revision = str_copy(src1->edit_number);
	    src2->edit->encoding = history_version_encoding_none;

	    src2->edit_origin =
                (history_version_ty *)history_version_type.alloc();
	    src2->edit_origin->errpos = str_copy(src1->errpos);
	    src2->edit_origin->revision = str_copy(src1->edit_number);
	    src2->edit_origin->encoding = history_version_encoding_none;
	}
	if (src1->move)
	    src2->move = str_copy(src1->move);

	//
	// index to track also
	//
	symtab_assign(cp->fstate_stp, src2->file_name, src2);
	if (src2->uuid && (!src2->move || src2->action != file_action_remove))
	    symtab_assign(cp->fstate_uuid_stp, src2->uuid, src2);
    }

    //
    // now release the src field of the cstate file
    //
    cstate_src_list_type.free(cp->cstate_data->src);
    cp->cstate_data->src = 0;
    trace(("}\n"));
}


cstate_ty *
change::cstate_get()
{
    trace(("change::cstate_get(this = %08lX)\n{\n", (long)this));
    assert(reference_count >= 1);
    change_lock_sync(this);
    if (!cstate_data)
    {
        string_ty *fn = change_cstate_filename_get(this);
	change_become(this);
	cstate_data = cstate_read_file(fn);
	change_become_undo(this);
	if (!cstate_data->brief_description)
	{
	    assert(cstate_data->errpos);
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", cstate_data->errpos);
	    sc.var_set_charstar("FieLD_Name", "brief_description");
	    change_fatal
	    (
		this,
		&sc,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	if (!cstate_data->description)
	{
	    assert(cstate_data->errpos);
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", cstate_data->errpos);
	    sc.var_set_charstar("FieLD_Name", "description");
	    change_fatal
	    (
		this,
		&sc,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	if (!(cstate_data->mask & cstate_state_mask))
	{
	    assert(cstate_data->errpos);
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", cstate_data->errpos);
	    sc.var_set_charstar("FieLD_Name", "state");
	    change_fatal
	    (
		this,
		&sc,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	if
	(
	    cstate_data->state >= cstate_state_being_developed
	&&
	    cstate_data->state <= cstate_state_being_integrated
	&&
	    !cstate_data->development_directory
	)
	{
	    assert(cstate_data->errpos);
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", cstate_data->errpos);
	    sc.var_set_charstar("FieLD_Name", "development_directory");
	    change_fatal
	    (
		this,
		&sc,
		i18n("$filename: contains no \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	if
	(
	    cstate_data->state == cstate_state_being_integrated
	&&
	    !cstate_data->integration_directory
	)
	{
	    assert(cstate_data->errpos);
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", cstate_data->errpos);
	    sc.var_set_charstar("FieLD_Name", "integration_directory");
	    change_fatal
	    (
		this,
		&sc,
		i18n("$filename: contains no \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	if
	(
	    cstate_data->state == cstate_state_completed
	&&
	    !cstate_data->delta_number
	)
	{
	    assert(cstate_data->errpos);
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", cstate_data->errpos);
	    sc.var_set_charstar("FieLD_Name", "delta_number");
	    change_fatal
	    (
		this,
		&sc,
		i18n("$filename: contains no \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	if (cstate_data->src)
	{
	    //
	    // convert from old format to new fstate format
	    //
	    assert(!fstate_data);
	    cstate_to_fstate(this);
	}
	change_cstate_improve(cstate_data);
    }
    trace(("return %08lX;\n", (long)cstate_data));
    trace(("}\n"));
    return cstate_data;
}
