//
//	aegis - project change supervisor
//	Copyright (C) 2000-2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate batchs
//

#include <change.h>
#include <change/env_set.h>
#include <change/file.h>
#include <change/test/batch.h>
#include <change/test/batch_result.h>
#include <error.h> // for assert
#include <fstate.h>
#include <os.h>
#include <pconf.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <tstrslt.h>
#include <undo.h>
#include <user.h>


batch_result_list_ty *
change_test_batch(change_ty *cp, string_list_ty *wlp, user_ty *up,
    bool baseline_flag, int current, int total)
{
    sub_context_ty  *scp;
    int		    flags;
    batch_result_list_ty *result;
    tstrslt_ty	    *tstrslt_data;
    string_ty	    *the_command;
    string_ty	    *s;
    string_ty	    *output_file_name;
    pconf_ty        *pconf_data;
    size_t	    j;
    size_t          k;
    string_ty	    *dir;

    trace(("change_test_batch(cp = %08lX, wlp = %08lX, up = %08lX, "
	"baseline_flag = %d, current = %d, total = %d)\n{\n", (long)cp,
	(long)wlp, (long)up, baseline_flag, current, total));
    pconf_data = change_pconf_get(cp, 1);
    the_command = pconf_data->batch_test_command;
    assert(the_command);

    //
    // resolve the file names
    //
    trace(("mark\n"));
    string_list_ty wl2;
    for (j = 0; j < wlp->nstrings; ++j)
    {
	string_ty	*fn;
	string_ty	*fn_abs;
	fstate_src_ty   *src_data;

	fn = wlp->string[j];
	src_data = change_file_find(cp, fn, view_path_first);
	if (src_data)
	{
	    fn_abs = change_file_path(cp, fn);
	}
	else
	{
	    src_data = project_file_find(cp->pp, fn, view_path_simple);
	    assert(src_data);
	    fn_abs = project_file_path(cp->pp, fn);
	}
	assert(fn_abs);
	wl2.push_back(fn_abs);
	str_free(fn_abs);
    }

    trace(("mark\n"));
    assert(cp->reference_count>=1);
    scp = sub_context_new();
    s = wl2.unsplit();
    sub_var_set_string(scp, "File_Names", s);
    str_free(s);
    trace(("mark\n"));
    output_file_name = os_edit_filename(0);
    sub_var_set_string(scp, "Output", output_file_name);
    if (baseline_flag && !cp->bogus)
    {
	string_list_ty spbl;
	project_search_path_get(cp->pp, &spbl, 0);
	s = spbl.unsplit(":");
	sub_var_set_string(scp, "Search_Path_Executable", s);
	str_free(s);
	sub_var_override(scp, "Search_Path_Executable");
	sub_var_optional(scp, "Search_Path_Executable");
    }
    sub_var_set_long(scp, "Current", current);
    sub_var_optional(scp, "Current");
    sub_var_set_long(scp, "Total", total);
    sub_var_optional(scp, "Total");
    user_become(up);
    undo_unlink_errok(output_file_name);
    user_become_undo();
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    //
    // we need input if any of the tests are manual
    //
    trace(("mark\n"));
    flags = OS_EXEC_FLAG_NO_INPUT;
    for (j = 0; j < wlp->nstrings; ++j)
    {
	string_ty	*file_name;
	fstate_src_ty   *src_data;

	file_name = wlp->string[j];
	src_data = change_file_find(cp, file_name, view_path_first);
	if (!src_data)
	    src_data = project_file_find(cp->pp, file_name, view_path_simple);
	assert(src_data);
	if (!src_data)
	    continue;
	switch (src_data->usage)
	{
	case file_usage_source:
	case file_usage_config:
	case file_usage_build:
	case file_usage_test:
#ifndef DEBUG
	default:
#endif
	    continue;

	case file_usage_manual_test:
	    flags = OS_EXEC_FLAG_INPUT;
	    break;
	}
	break;
    }

    //
    // directory depends on the state of the change
    //
    // During long tests the automounter can unmount the
    // directory referenced by the ``dir'' variable.
    // To minimize this, it is essential that they are
    // unresolved, and thus always trigger the automounter.
    //
    trace(("mark\n"));
    dir = project_baseline_path_get(cp->pp, 0);
    if (!baseline_flag && !cp->bogus)
    {
	cstate_ty       *cstate_data;

	cstate_data = change_cstate_get(cp);
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	    assert(0);

	case cstate_state_completed:
	    break;

	case cstate_state_being_integrated:
	    dir = change_integration_directory_get(cp, 0);
	    break;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	    dir = change_development_directory_get(cp, 0);
	    break;
	}
    }

    //
    // display progress message if requested
    //
    if (total > 0)
    {
	sub_context_ty sc;
	sc.var_set_long("Current", current + 1);
	sc.var_set_long("Total", total);
	if (wlp->nstrings == 1)
	    change_error(cp, &sc, i18n("test $current of $total"));
	else
	{
	    sc.var_set_long("Last", current + wlp->nstrings);
	    change_error
	    (
		cp,
		&sc,
		i18n("batch test from $current to $last of $total")
	    );
	}
    }

    //
    // run the command
    //	    This tests all of the files at once.
    //
    trace(("mark\n"));
    change_env_set(cp, 1);
    user_become(up);
    os_execute(the_command, flags, dir);
    str_free(the_command);

    //
    // read the output
    //
    trace(("mark\n"));
    tstrslt_data = tstrslt_read_file(output_file_name);
    os_unlink_errok(output_file_name);
    os_become_undo();
    if (!tstrslt_data->test_result)
    {
	tstrslt_data->test_result =
	    (tstrslt_test_result_list_ty *)
            tstrslt_test_result_list_type.alloc();
    }

    //
    // transcribe the result structure
    //
    trace(("mark\n"));
    result = batch_result_list_new();
    for (j = 0; j < tstrslt_data->test_result->length; ++j)
    {
	tstrslt_test_result_ty *p;

	//
	// perform sanity checks
	//
	p = tstrslt_data->test_result->list[j];
	if (!p->file_name)
	{
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", output_file_name);
	    sc.var_set_charstar("FieLD_Name", "test_result.file_name");
	    sc.var_set_charstar("REASON", " (no file_name field)");
	    sc.var_append_if_unused("REASON");
	    change_fatal
	    (
		cp,
		&sc,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	}
	for (k = 0; k < wlp->nstrings; ++k)
	{
	    if (str_equal(p->file_name, wl2.string[k]))
	    {
		//
		// map abs name to relative name
		//
		str_free(p->file_name);
		p->file_name = str_copy(wlp->string[k]);
		break;
	    }
	}
	if (!wlp->member(p->file_name))
	{
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", output_file_name);
	    sc.var_set_charstar("FieLD_Name", "test_result.file_name");
	    sc.var_set_string
    	    (
		"REASON",
		" (filename \"" + nstring(p->file_name) + "\" not in the list)"
	    );
	    sc.var_append_if_unused("REASON");
	    change_fatal
	    (
		cp,
		&sc,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	}

	if (batch_result_list_member(result, p->file_name, p->architecture))
	{
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", output_file_name);
	    sc.var_set_charstar("FieLD_Name", "test_result.file_name");
	    sc.var_set_string
    	    (
		"REASON",
		(
		    " (filename \""
		+
		    nstring(p->file_name)
		+
		    "\" named more than once)"
		)
	    );
	    sc.var_append_if_unused("REASON");
	    change_fatal
	    (
		cp,
		&sc,
		i18n("$filename: corrupted \"$field_name\" field")
	    );
	    // NOTREACHED
	}

	//
	// add result to list
	//
	batch_result_list_append
	(
	    result,
	    p->file_name,
	    p->exit_status,
	    p->architecture
	);

	//
	// emit verbose messages
	//
	switch (p->exit_status)
	{
	case 1:
	    if (baseline_flag)
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", p->file_name);
		if (p->architecture)
		{
		    sc.var_set_string("ARCHitecture", p->architecture);
		    sc.var_override("ARCHitecture");
		    change_verbose
	    	    (
			cp,
			&sc,
			i18n
			(
		     "$filename baseline fail, architecture $architecture, good"
			)
		    );
		}
		else
		{
		    change_verbose
	    	    (
			cp,
			&sc,
			i18n("$filename baseline fail, good")
		    );
		}
		result->pass_count++;
	    }
	    else
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", p->file_name);
		if (p->architecture)
		{
		    sc.var_set_string("ARCHitecture", p->architecture);
		    sc.var_override("ARCHitecture");
		    change_verbose
		    (
			cp,
			&sc,
			i18n("$filename fail, architecture $architecture")
		    );
		}
		else
		    change_verbose(cp, &sc, i18n("$filename fail"));
		result->fail_count++;
	    }
	    break;

	case 0:
	    if (baseline_flag)
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", p->file_name);
		if (p->architecture)
		{
		    sc.var_set_string("ARCHitecture", p->architecture);
		    sc.var_override("ARCHitecture");
		    change_verbose
		    (
			cp,
			&sc,
			i18n
			(
		 "$filename baseline pass, architecture $architecture, not good"
			)
		    );
		}
		else
		{
		    change_verbose
		    (
			cp,
			&sc,
			i18n("$filename baseline pass, not good")
		    );
		}
		result->fail_count++;
	    }
	    else
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", p->file_name);
		if (p->architecture)
		{
		    sc.var_set_string("ARCHitecture", p->architecture);
		    sc.var_override("ARCHitecture");
		    change_verbose
	    	    (
			cp,
			&sc,
			i18n("$filename pass, architecture $architecture")
		    );
		}
		else
		    change_verbose(cp, &sc, i18n("$filename pass"));
		result->pass_count++;
	    }
	    break;

	default:
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", p->file_name);
		if (p->architecture)
		{
		    sc.var_set_string("ARCHitecture", p->architecture);
		    sc.var_override("ARCHitecture");
		    change_verbose
	    	    (
			cp,
			&sc,
			i18n("$filename no result, architecture $architecture")
		    );
		}
		else
		    change_verbose(cp, &sc, i18n("$filename no result"));
		result->no_result_count++;
	    }
	    break;
	}
    }
    tstrslt_type.free(tstrslt_data);
    str_free(output_file_name);

    //
    // all done
    //
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
