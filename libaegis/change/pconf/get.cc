//
//	aegis - project change supervisor
//	Copyright (C) 1999-2004 Peter Miller;
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
// MANIFEST: functions to manipulate gets
//

#include <attrlistveri.h>
#include <change.h>
#include <change/file.h>
#include <change/lock_sync.h>
#include <error.h>	// for assert
#include <fstate.h>
#include <input/catenate.h>
#include <input/crlf.h>
#include <input/file.h>
#include <mem.h>
#include <os.h>
#include <os/isa/path_prefix.h>
#include <project/file.h>
#include <str_list.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>


static void
pconf_improve(change_ty *cp)
{
    pconf_ty        *d;
    sub_context_ty  *scp;
    size_t          j;

    d = cp->pconf_data;
    if (!d->build_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "build_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!d->development_build_command)
	d->development_build_command = str_copy(d->build_command);

    // This field is obsolete.
    if (d->mask & pconf_create_symlinks_before_build_mask)
    {
	if (change_is_being_developed(cp))
	{
	    change_warning_obsolete_field
	    (
		cp,
		d->errpos,
		"create_symlinks_before_build",
		"development_directory_style.source_file_symlink"
	    );
	}
    }

    // This field is obsolete.
    if (d->mask & pconf_remove_symlinks_after_build_mask)
    {
	if (change_is_being_developed(cp))
	{
	    change_warning_obsolete_field
	    (
		cp,
		d->errpos,
		"remove_symlinks_after_build",
		"development_directory_style.during_build_only"
	    );
	}
    }

    // This field is obsolete.
    if (d->mask & pconf_create_symlinks_before_integration_build_mask)
    {
	if (change_is_being_developed(cp))
	{
	    change_warning_obsolete_field
	    (
		cp,
		d->errpos,
		"create_symlinks_before_integration_build",
		"integration_directory_style.source_file_symlink"
	    );
	}
    }
    else
    {
	d->create_symlinks_before_integration_build =
	    d->create_symlinks_before_build;
	if (d->mask & pconf_create_symlinks_before_build_mask)
	    d->mask |= pconf_create_symlinks_before_integration_build_mask;
    }

    // This field is obsolete.
    if (d->mask & pconf_remove_symlinks_after_integration_build_mask)
    {
	if (change_is_being_developed(cp))
	{
	    change_warning_obsolete_field
	    (
		cp,
		d->errpos,
		"remove_symlinks_after_integration_build",
		"integration_directory_style.during_build_only"
	    );
	}
    }
    else
    {
	//
	// Integration builds always remove the symlinks
	// again, even if they are kept around in the
	// development directories.  This stops them
	// becoming stale if there are deeper baseline
	// integrations.
	//
	d->remove_symlinks_after_integration_build = true;
    }
    if (!d->history_create_command && d->history_put_command)
	d->history_create_command = str_copy(d->history_put_command);
    if (!d->history_create_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "history_create_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!d->history_get_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "history_get_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!d->history_put_command && d->history_create_command)
	d->history_put_command = str_copy(d->history_create_command);
    if (!d->history_put_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "history_put_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!d->history_query_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "history_query_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!d->diff_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "diff_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!d->diff3_command && !d->merge_command)
    {
	assert(d->errpos);
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", d->errpos);
	sub_var_set_charstar(scp, "FieLD_Name", "merge_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp);
    }
    if (!(d->mask & pconf_shell_safe_filenames_mask))
    {
	d->shell_safe_filenames = true;
	d->mask |= pconf_shell_safe_filenames_mask;
    }
    if (d->file_template)
    {
	for (j = 0; j < d->file_template->length; ++j)
	{
	    pconf_file_template_ty *tp;

	    tp = d->file_template->list[j];
	    if (!tp->pattern || !tp->pattern->length)
	    {
		assert(tp->errpos);
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", tp->errpos);
		sub_var_set_charstar
		(
		    scp,
		    "FieLD_Name",
		    "file_template.pattern"
		);
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: contains no \"$field_name\" field")
		);
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    if (!tp->body && !tp->body_command)
	    {
		assert(tp->errpos);
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", tp->errpos);
		sub_var_set_charstar(scp, "FieLD_Name", "file_template.body");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: contains no \"$field_name\" field")
		);
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    if (tp->body && tp->body_command)
	    {
		assert(tp->errpos);
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", tp->errpos);
		sub_var_set_charstar(scp, "FieLD_Name", "file_template.body");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: corrupted \"$field_name\" field")
		);
		// NOTREACHED
		sub_context_delete(scp);
	    }
	}
    }
    attributes_list_verify(d->project_specific);

    if (d->change_file_command)
    {
	if (!d->change_file_undo_command)
	    d->change_file_undo_command = str_copy(d->change_file_command);
	if (!d->new_file_command)
	    d->new_file_command = str_copy(d->change_file_command);
	if (!d->new_test_command)
	    d->new_test_command = str_copy(d->change_file_command);
	if (!d->copy_file_command)
	    d->copy_file_command = str_copy(d->change_file_command);
	if (!d->remove_file_command)
	    d->remove_file_command = str_copy(d->change_file_command);
    }
    if (d->change_file_undo_command)
    {
	if (!d->new_file_undo_command)
	    d->new_file_undo_command = str_copy(d->change_file_undo_command);
	if (!d->new_test_undo_command)
	    d->new_test_undo_command = str_copy(d->change_file_undo_command);
	if (!d->copy_file_undo_command)
	    d->copy_file_undo_command = str_copy(d->change_file_undo_command);
	if (!d->remove_file_undo_command)
	    d->remove_file_undo_command = str_copy(d->change_file_undo_command);
    }

    // See also pconf_improve_more(), below.
}


static void
set_pconf_symlink_exceptions_defaults(pconf_ty *pconf_data)
{
    type_ty         *type_p;
    string_ty       **str_p;

    //
    // make sure the list is there
    //
    if (!pconf_data->symlink_exceptions)
    {
	pconf_data->symlink_exceptions =
	    (pconf_symlink_exceptions_list_ty *)
	    pconf_symlink_exceptions_list_type.alloc();
    }

    //
    // append the logfile to the list
    //
    str_p =
	(string_ty **)
	pconf_symlink_exceptions_list_type.list_parse
	(
	    pconf_data->symlink_exceptions,
	    &type_p
	);
    assert(type_p == &string_type);
    *str_p = str_copy(change_logfile_basename());
}


static input_ty *
input_catenate_tricky(string_list_ty *filename)
{
    input_ty        **fpl;
    size_t          j;
    input_ty        *fp;

    assert(filename->nstrings >= 1);
    fpl = (input_ty **)mem_alloc(filename->nstrings * sizeof(fpl[0]));
    for (j = 0; j < filename->nstrings; ++j)
    {
	fpl[j] = input_crlf(input_file_open(filename->string[j]), 1);
    }
    fp = input_catenate(fpl, filename->nstrings, 1);
    mem_free(fpl);
    return fp;
}


static int
candidate(fstate_src_ty *src)
{
    if (src->about_to_be_created_by)
	return 0;
    if (src->deleted_by)
	return 0;
    if (src->about_to_be_copied_by)
	return 0;
    switch (src->usage)
    {
    case file_usage_source:
    case file_usage_config:
	switch (src->action)
	{
	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
	    return 1;

	case file_action_remove:
	    break;
	}
	break;

    case file_usage_build:
    case file_usage_test:
    case file_usage_manual_test:
	break;
    }
    return 0;
}


static pconf_ty *
pconf_read_by_usage(change_ty *cp)
{
    string_ty       *s;
    input_ty        *ifp;
    string_list_ty  filename;
    size_t          j;
    fstate_src_ty   *src;
    pconf_ty        *result;
    string_ty       *dir;

    //
    // Build the list of files.
    //
    // We use a symbol table so that we get O(n) behaviour.
    // (The string_list_append_list_unique is O(n**2), oops.)
    //
    string_list_constructor(&filename);
    for (j = 0;; ++j)
    {
	src = change_file_nth(cp, j, view_path_extreme);
	if (!src)
	    break;
	if (src->usage != file_usage_config)
	    continue;
	if (!candidate(src))
	    continue;
	s = change_file_path(cp, src->file_name);
	if (!s)
	    s = project_file_path(cp->pp, src->file_name);
	assert(s);
	string_list_append(&filename, s);
	str_free(s);
    }

    //
    // If there are no candidate files,
    // look for files with the right name
    // (this is the way Aegis used to do it).
    //
    if (filename.nstrings < 1)
    {
	s = str_from_c(THE_CONFIG_FILE_NEW);
	src = change_file_find(cp, s, view_path_extreme);
	str_free(s);

	if (!src)
	{
	    s = str_from_c(THE_CONFIG_FILE_OLD);
	    src = change_file_find(cp, s, view_path_extreme);
	    str_free(s);
	}

	if (src)
	{
	    s = change_file_path(cp, src->file_name);
	    if (!s)
		s = project_file_path(cp->pp, src->file_name);
	    assert(s);
	    string_list_append(&filename, s);
	    str_free(s);

	    //
	    // This is a hack, but it makes transition from the old
	    // method to the new method easier.  We change the file have
	    // "config" usage.  At some point the file state will be
	    // written out again, and the file will thus be quietly
	    // converted to have "config" usage.
	    //
	    src->usage = file_usage_config;
	}
    }

    //
    // If there are no candidate files,
    // return a NULL pointer.
    //
    if (filename.nstrings < 1)
    {
	string_list_destructor(&filename);
	return 0;
    }

    //
    // Read the configuration information.
    //
    change_become(cp);
    ifp = input_catenate_tricky(&filename);
    result = (pconf_ty *)parse_input(ifp, &pconf_type);
    // as a side-effect, parse_input will delete fp
    change_become_undo();

    //
    // If there is a configuration directory specified, then all the
    // source files in that directory are also config files.  We need
    // to extend the list and read the catenate config again.
    //
    // But... aenf usually marks these files as config files, so there
    // is a very good chance that this loop will be very fast and find
    // nothing else to add to the file list, in which case we *dont't*
    // re-read the project config.
    //
    dir = result->configuration_directory;
    if (dir)
    {
	int             more;

	more = 0;
	for (j = 0;; ++j)
	{
	    src = change_file_nth(cp, j, view_path_extreme);
	    if (!src)
		break;
	    if (src->usage == file_usage_config)
		continue;
	    if (!candidate(src))
		continue;
	    if (!os_isa_path_prefix(dir, src->file_name))
		continue;

	    s = change_file_path(cp, src->file_name);
	    if (!s)
		s = project_file_path(cp->pp, src->file_name);
	    assert(s);
	    string_list_append(&filename, s);
	    str_free(s);
	    more = 1;

	    //
	    // This is a hack, but it makes transition from the old
	    // method to the new method easier.  We change the file have
	    // "config" usage.	At some point the file state will be
	    // written out again, and the file will thus be quietly
	    // converted to have "config" usage.
	    //
	    src->usage = file_usage_config;
	}

	//
	// Only re-read of the list of files has been extended.
	//
	if (more)
	{
	    pconf_type.free((void *)result);

	    change_become(cp);
	    ifp = input_catenate_tricky(&filename);
	    result = (pconf_ty *)parse_input(ifp, &pconf_type);
	    // as a side-effect, parse_input will delete fp
	    change_become_undo();
	}
    }
    string_list_destructor(&filename);

    //
    // Report success.
    //
    return result;
}


static void
pconf_improve_more(change_ty *cp)
{
    size_t          j;
    static string_ty *star_comma_d;

    //
    // set the architecture default
    //
    pconf_ty *d = cp->pconf_data;
    assert(d);
    if (!d->architecture)
    {
	d->architecture =
	    (pconf_architecture_list_ty *)
	    pconf_architecture_list_type.alloc();
    }
    if (!d->architecture->length)
    {
	type_ty         *type_p;
	pconf_architecture_ty **app;
	pconf_architecture_ty *ap;

	app =
	    (pconf_architecture_ty **)
	    pconf_architecture_list_type.list_parse
	    (
		d->architecture,
		&type_p
	    );
	assert(type_p == &pconf_architecture_type);
	ap = (pconf_architecture_ty *)pconf_architecture_type.alloc();
	*app = ap;
	ap->name = str_from_c("unspecified");
	ap->pattern = str_from_c("*");
    }
    for (j = 0; j < d->architecture->length; ++j)
    {
	pconf_architecture_ty *ap;

	ap = d->architecture->list[j];
	if (!ap->name || !ap->pattern)
	{
	    sub_context_ty *scp;

	    assert(ap->errpos);
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", ap->errpos);
	    change_fatal
	    (
		cp,
		scp,
		i18n("$filename: arch needs name and pattern")
	    );
	    // NOTREACHED
	    sub_context_delete(scp);
	}

	//
	// Make sure the report generator can see it.
	//
	ap->mask |= pconf_architecture_mode_mask;
    }

    //
    // set the maximum_filename_length default
    //
    if (d->mask & pconf_maximum_filename_length_mask)
    {
	if (d->maximum_filename_length < 9)
	    d->maximum_filename_length = 9;
	if (d->maximum_filename_length > 255)
	    d->maximum_filename_length = 255;
    }
    else
	d->maximum_filename_length = 255;

    //
    // set the filename_pattern_accept default
    //
    if (!d->filename_pattern_accept)
    {
	d->filename_pattern_accept =
	    (pconf_filename_pattern_accept_list_ty *)
	    pconf_filename_pattern_accept_list_type.alloc();
    }
    if (!d->filename_pattern_accept->length)
    {
	type_ty         *type_p;
	string_ty       **addr_p;

	addr_p =
	    (string_ty **)
	    pconf_filename_pattern_accept_list_type.list_parse
	    (
		d->filename_pattern_accept,
		&type_p
	    );
	assert(type_p == &string_type);
	*addr_p = str_from_c("*");
    }
    if (!star_comma_d)
	star_comma_d = str_from_c("*,D");
    if (!d->filename_pattern_reject)
    {
	d->filename_pattern_reject =
	    (pconf_filename_pattern_reject_list_ty *)
	    pconf_filename_pattern_reject_list_type.alloc();
    }
    for (j = 0; j < d->filename_pattern_reject->length; ++j)
    {
	if
	(
	    str_equal
	    (
		star_comma_d,
		d->filename_pattern_reject->list[j]
	    )
	)
	    break;
    }
    if (j >= d->filename_pattern_reject->length)
    {
	type_ty         *type_p;
	string_ty       **addr_p;

	addr_p =
	    (string_ty **)
	    pconf_filename_pattern_reject_list_type.list_parse
	    (
		d->filename_pattern_reject,
		&type_p
	    );
	assert(type_p == &string_type);
	*addr_p = str_copy(star_comma_d);
    }

    //
    // make sure symlink_exceptions is there, even if empty
    //
    set_pconf_symlink_exceptions_defaults(d);

    //
    // set the test_command default
    //
    if (!d->test_command)
	d->test_command = str_from_c("$shell $file_name");
    if (!d->development_test_command)
	d->development_test_command =
	    str_copy(d->test_command);

    //
    // set the development directory template default
    //
    if (!d->development_directory_template)
    {
	d->development_directory_template =
	    str_from_c
	    (
		"$ddd/${left $project ${expr ${namemax $ddd} - ${length "
		".$magic${zpad $c 3}}}}.$magic${zpad $c 3}"
	    );
    }

    //
    // Set the test filename template
    //
    if (!d->new_test_filename)
    {
	d->new_test_filename =
	    str_from_c
	    (
		"test/${zpad $hundred 2}/t${zpad $number 4}${left $type 1}.sh"
	    );
    }

    //
    // Make sure the report generator can see the enums.
    //
    d->mask |=
	pconf_history_put_trashes_file_mask |
	pconf_history_content_limitation_mask
	;

    //
    // Default settings for the work area styles.
    //
    if (!d->development_directory_style)
    {
	d->development_directory_style =
	    (work_area_style_ty *)work_area_style_type.alloc();
	d->development_directory_style->source_file_link = false;
	d->development_directory_style->source_file_symlink =
	    d->create_symlinks_before_build;
	d->development_directory_style->source_file_copy = false;
	d->development_directory_style->derived_file_link = false;
	d->development_directory_style->derived_file_symlink =
	    d->create_symlinks_before_build;
	d->development_directory_style->derived_file_copy = false;
	d->development_directory_style->during_build_only =
	    d->remove_symlinks_after_build;
	if (d->mask & pconf_create_symlinks_before_build_mask)
	{
	    d->development_directory_style->mask |=
	    	work_area_style_source_file_symlink_mask |
	    	work_area_style_derived_file_symlink_mask
	    	;
	}
	if (d->mask & pconf_remove_symlinks_after_build_mask)
	{
	    d->development_directory_style->mask |=
	    	work_area_style_during_build_only_mask
	    	;
	}
    }
    if
    (
	!(
	    d->development_directory_style->mask
	&
	    work_area_style_source_file_whiteout_mask
	)
    )
    {
	//
        // You only need whiteout files if you don't have some kind of
        // mirror in the development directory.
	//
	d->development_directory_style->source_file_whiteout =
	    !(
		d->development_directory_style->source_file_link
	    ||
		d->development_directory_style->source_file_symlink
	    ||
		d->development_directory_style->source_file_copy
	    );
	d->development_directory_style->mask |=
	    work_area_style_source_file_whiteout_mask;
    }

    if (!d->integration_directory_style)
    {
	//
        // We want the integration directory to behave the same as the
        // development directory.
	//
	d->integration_directory_style =
	    work_area_style_copy(d->development_directory_style);

	//
        // Unless they have explicity used obsolete fields to change
        // things around.
	//
	if (d->mask & pconf_create_symlinks_before_integration_build_mask)
	{
	    d->integration_directory_style->source_file_symlink =
		d->create_symlinks_before_integration_build;
	    d->integration_directory_style->derived_file_symlink =
		d->create_symlinks_before_integration_build;
	}
	if (d->mask & pconf_remove_symlinks_after_integration_build_mask)
	{
	    d->integration_directory_style->during_build_only =
		d->remove_symlinks_after_integration_build;
	}
    }
    if
    (
	!(
	    d->integration_directory_style->mask
	&
	    work_area_style_source_file_whiteout_mask
	)
    )
    {
	//
        // Omit whiteout from the integration directory.  The developer
        // is supposed to have removed all sign of it already.
	//
	d->integration_directory_style->source_file_whiteout = false;
	d->integration_directory_style->mask |=
	    work_area_style_source_file_whiteout_mask;
    }
}


pconf_ty *
change_pconf_get(change_ty *cp, int required)
{

    trace(("change_pconf_get(cp = %8.8lX, required = %d)\n{\n", (long)cp,
	required));
    assert(cp->reference_count >= 1);
    change_lock_sync(cp);
    if (!cp->pconf_data)
    {

	cp->pconf_data = pconf_read_by_usage(cp);
	if (!cp->pconf_data)
	{
	    if (required)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "File_Name", THE_CONFIG_FILE_NEW);
		change_fatal
		(
		    cp,
		    scp,
		    i18n("you must create a \"$filename\" file")
		);
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    cp->pconf_data = (pconf_ty *)pconf_type.alloc();
	    cp->pconf_data->errpos = str_from_c(THE_CONFIG_FILE_NEW);
	    cp->pconf_data->shell_safe_filenames = true;
	}

	if (required)
	    pconf_improve(cp);

	pconf_improve_more(cp);
    }
    trace(("return %08lX;\n", (long)cp->pconf_data));
    trace(("}\n"));
    return cp->pconf_data;
}
