//
//	aegis - project change supervisor
//	Copyright (C) 2000-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/format_elpsd.h>
#include <common/now.h>
#include <common/nstring/list.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/test/batch.h>
#include <libaegis/change/test/batch_result.h>
#include <libaegis/fstate.h>
#include <libaegis/os.h>
#include <libaegis/pconf.h>
#include <libaegis/project/file.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/tstrslt.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>


batch_result_list_ty *
change_test_batch(change::pointer cp, string_list_ty *wlp, user_ty::pointer up,
    bool baseline_flag, int current, int total,
    const nstring_list &variable_assignments, const long *remaining)
{
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
	(long)wlp, (long)up.get(), baseline_flag, current, total));
    pconf_data = change_pconf_get(cp, 1);
    the_command = pconf_data->batch_test_command;
    assert(the_command);

    //
    // resolve the file names
    //
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

    assert(cp->reference_count>=1);
    s = wl2.unsplit();
    sub_context_ty sc;
    sc.var_set_string("File_Names", s);
    str_free(s);
    output_file_name = os_edit_filename(0);
    sc.var_set_string("Output", output_file_name);
    if (baseline_flag && !cp->bogus)
    {
	string_list_ty spbl;
	project_search_path_get(cp->pp, &spbl, 0);
	s = spbl.unsplit(":");
	sc.var_set_string("Search_Path_Executable", s);
	str_free(s);
	sc.var_override("Search_Path_Executable");
	sc.var_optional("Search_Path_Executable");
    }
    sc.var_set_long("Current", current);
    sc.var_optional("Current");
    sc.var_set_long("Total", total);
    sc.var_optional("Total");
    sc.var_set_string("REMaining", format_elapsed(remaining[0]));
    sc.var_optional("REMaining");

    // Quote the variable assignments
    nstring_list var;
    for (size_t jj = 0; jj < variable_assignments.size(); ++jj)
	var.push_back(variable_assignments[jj].quote_shell());
    sc.var_set_string("VARiables", var.unsplit());
    sc.var_append_if_unused("VARiables");

    up->become_begin();
    undo_unlink_errok(output_file_name);
    up->become_end();
    the_command = sc.substitute(cp, the_command);

    //
    // we need input if any of the tests are manual
    //
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
    // directory referenced by the "dir" variable.
    // To minimize this, it is essential that they are
    // unresolved, and thus always trigger the automounter.
    //
    dir = cp->pp->baseline_path_get();
    trace(("dir = \"%s\";\n", dir->str_text));
    if (!baseline_flag && !cp->bogus)
    {
	cstate_ty       *cstate_data;

	cstate_data = cp->cstate_get();
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	    assert(0);

	case cstate_state_completed:
	    break;

	case cstate_state_being_integrated:
	    dir = change_integration_directory_get(cp, 0);
	    trace(("dir = \"%s\";\n", dir->str_text));
	    break;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	    dir = change_development_directory_get(cp, 0);
	    trace(("dir = \"%s\";\n", dir->str_text));
	    break;
	}
    }

    //
    // display progress message if requested
    //
    if (total > 0)
    {
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
    // we average the elapsed time over all tests
    //
    change_env_set(cp, 1);
    up->become_begin();
    double t_begin = dtime();
    os_execute(the_command, flags, dir);
    double t_end = dtime();
    str_free(the_command);
    double elapsed = t_end - t_begin;
    elapsed /= wlp->nstrings;
    trace(("elapsed = %g\n", elapsed));

    //
    // read the output
    //
    tstrslt_data = tstrslt_read_file(output_file_name);
    os_unlink_errok(output_file_name);
    up->become_end();
    if (!tstrslt_data->test_result)
    {
	tstrslt_data->test_result =
	    (tstrslt_test_result_list_ty *)
            tstrslt_test_result_list_type.alloc();
    }

    //
    // transcribe the result structure
    //
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
	    p->architecture,
            elapsed
	);

	//
	// emit verbose messages
	//
	switch (p->exit_status)
	{
	case 1:
	    if (baseline_flag)
	    {
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

	case 77:
	    {
                // Note: the value 77 was chosen to be compatible with
                // other test systems.
		sc.var_set_string("File_Name", p->file_name);
		if (p->architecture)
		{
		    sc.var_set_string("ARCHitecture", p->architecture);
		    sc.var_override("ARCHitecture");
		    change_verbose
	    	    (
			cp,
			&sc,
			i18n("$filename skipped, architecture $architecture")
		    );
		}
		else
		    change_verbose(cp, &sc, i18n("$filename skipped"));
		result->skip_count++;
	    }
	    break;

	default:
	    {
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
