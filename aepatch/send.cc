//
//	aegis - project change supervisor
//	Copyright (C) 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate sends
//

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <error.h>
#include <gettime.h>
#include <help.h>
#include <input/file.h>
#include <now.h>
#include <option.h>
#include <os.h>
#include <output/conten_encod.h>
#include <output/base64.h>
#include <output/file.h>
#include <output/gzip.h>
#include <output/prefix.h>
#include <output/wrap.h>
#include <progname.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <project.h>
#include <project/history.h>
#include <send.h>
#include <str.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <undo.h>


#define NO_TIME_SET ((time_t)(-1))


static int
len_printable(string_ty *s, int max)
{
    const char      *cp;
    int             result;

    if (!s)
	return 0;
    // Intentionally the C locale, not the user's locale
    for (cp = s->str_text; *cp && isprint((unsigned char)*cp); ++cp)
	;
    result = (cp - s->str_text);
    if (result > max)
	result = max;
    return result;
}


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static string_ty *
change_description_get(change_ty *cp)
{
    cstate_ty       *cstate_data;

    cstate_data = change_cstate_get(cp);
    return cstate_data->description;
}


void
patch_send(void)
{
    const char      *compatibility;
    string_ty       *original_filename = 0;
    int             original_filename_unlink;
    string_ty       *input_filename = 0;
    int             input_filename_unlink;
    string_ty       *output_file_name;
    output_ty       *t1;
    output_ty       *t2;
    string_ty       *project_name;
    long            change_number;
    const char      *branch;
    int             grandparent;
    int             trunk;
    output_ty       *ofp;
    input_ty        *ifp;
    project_ty      *pp;
    change_ty       *cp;
    user_ty         *up;
    cstate_ty       *cstate_data;
    string_ty       *output;
    string_ty       *s;
    string_ty       *s2;
    size_t          j;
    content_encoding_t ascii_armor;
    int             compress;
    string_ty       *dev_null;
    long            delta_number;
    time_t          delta_date;
    const char      *delta_name;
    int             use_meta_data;

    compatibility = 0;
    branch = 0;
    change_number = 0;
    grandparent = 0;
    project_name = 0;
    trunk = 0;
    output = 0;
    ascii_armor = content_encoding_unset;
    compress = -1;
    delta_date = NO_TIME_SET;
    delta_number = -1;
    delta_name = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_change:
	case arglex_token_delta_from_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, usage);
	    continue;

	case arglex_token_branch:
	    if (branch)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_branch, usage);

	    case arglex_token_number:
	    case arglex_token_string:
		branch = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_trunk:
	    if (trunk)
		duplicate_option(usage);
	    ++trunk;
	    break;

	case arglex_token_grandparent:
	    if (grandparent)
		duplicate_option(usage);
	    ++grandparent;
	    break;

	case arglex_token_output:
	    if (output)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_output, usage);
		// NOTREACHED

	    case arglex_token_stdio:
		output = str_from_c("");
		break;

	    case arglex_token_string:
		output = str_from_c(arglex_value.alv_string);
		break;
	    }
	    break;

	case arglex_token_ascii_armor:
	    if (ascii_armor != content_encoding_unset)
	    {
		duplicate_option_by_name
		(
		    arglex_token_content_transfer_encoding,
		    usage
		);
	    }
	    ascii_armor = content_encoding_base64;
	    break;

	case arglex_token_ascii_armor_not:
	    if (ascii_armor != content_encoding_unset)
	    {
		duplicate_option_by_name
		(
		    arglex_token_content_transfer_encoding,
		    usage
		);
	    }
	    ascii_armor = content_encoding_none;
	    break;

	case arglex_token_content_transfer_encoding:
	    if (ascii_armor != content_encoding_unset)
		duplicate_option(usage);
	    if (arglex() != arglex_token_string)
	    {
		option_needs_string
		(
		    arglex_token_content_transfer_encoding,
		    usage
		);
	    }
	    ascii_armor = content_encoding_grok(arglex_value.alv_string);
	    break;

	case arglex_token_compress:
	    if (compress > 0)
		duplicate_option(usage);
	    else if (compress >= 0)
	    {
	      compress_yuck:
		mutually_exclusive_options
		(
		    arglex_token_compress,
		    arglex_token_compress_not,
		    usage
		);
	    }
	    compress = 1;
	    break;

	case arglex_token_compress_not:
	    if (compress == 0)
		duplicate_option(usage);
	    else if (compress >= 0)
		goto compress_yuck;
	    compress = 0;
	    break;

	case arglex_token_delta:
	    if (delta_number >= 0 || delta_name)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_delta, usage);
		// NOTREACHED

	    case arglex_token_number:
		delta_number = arglex_value.alv_number;
		if (delta_number < 0)
		{
		    sub_context_ty *scp;

		    scp = sub_context_new();
		    sub_var_set_long(scp, "Number", delta_number);
		    fatal_intl(scp, i18n("delta $number out of range"));
		    // NOTREACHED
		    sub_context_delete(scp);
		}
		break;

	    case arglex_token_string:
		delta_name = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_delta_date:
	    if (delta_date != NO_TIME_SET)
		duplicate_option(usage);
	    if (arglex() != arglex_token_string)
	    {
		option_needs_string(arglex_token_delta_date, usage);
		// NOTREACHED
	    }
	    delta_date = date_scan(arglex_value.alv_string);
	    if (delta_date == NO_TIME_SET)
		fatal_date_unknown(arglex_value.alv_string);
	    break;

	case arglex_token_compatibility:
	    if (compatibility)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    case arglex_token_string:
	    case arglex_token_number:
		compatibility = arglex_value.alv_string;
		break;

	    default:
		option_needs_string(arglex_token_compatibility, usage);
		// NOTREACHED
	    }
	    break;
	}
	arglex();
    }

    //
    // reject illegal combinations of options
    //
    if (grandparent)
    {
	if (branch)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_grandparent,
		usage
	    );
	}
	if (trunk)
	{
	    mutually_exclusive_options
	    (
		arglex_token_trunk,
		arglex_token_grandparent,
		usage
	    );
	}
	branch = "..";
    }
    if (trunk)
    {
	if (branch)
	{
	    mutually_exclusive_options
	    (
		arglex_token_branch,
		arglex_token_trunk,
		usage
	    );
	}
	branch = "";
    }
    if
    (
	(
	    (delta_name || delta_number >= 0)
	+
	    !!change_number
	+
	    (delta_date != NO_TIME_SET)
	)
    >
	1
    )
    {
	mutually_exclusive_options3
	(
	    arglex_token_delta,
	    arglex_token_delta_date,
	    arglex_token_change,
	    usage
	);
    }

    //
    // Translate the compatibility version number into a set of
    // capabilities.
    //
    use_meta_data = 1;
    if (compatibility)
    {
	//
	// FIXME: should we check that it actually looks like a version
	// string?
	//

	//
	// Patch meta-data were introduced in Peter's 4.16.D081,
	// publicly in 4.17
	//
	use_meta_data = (strverscmp(compatibility, "4.17") >= 0);
    }

    //
    // locate project data
    //
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    //
    // locate the other branch
    //
    if (branch)
	pp = project_find_branch(pp, branch);

    //
    // locate user data
    //
    up = user_executing(pp);

    //
    // it is an error if the delta does not exist
    //
    if (delta_name)
    {
	string_ty	*s1;

	s1 = str_from_c(delta_name);
	change_number = project_history_change_by_name(pp, s1, 0);
	str_free(s1);
	delta_name = 0;
    }
    if (delta_date != NO_TIME_SET)
    {
	//
	// If the time is in the future, you could get a different
	// answer for the same input at some point in the future.
	//
	// This is the "time safe" quality first described by
	// Damon Poole <damon@ede.com>
	//
	if (delta_date > now())
	    project_error(pp, 0, i18n("date in the future"));

	//
    	// Now find the change number corresponding.
	//
	change_number = project_history_change_by_timestamp(pp, delta_date);
    }
    if (delta_number >= 0)
    {
	// does not return if no such delta number
	change_number = project_history_change_by_delta(pp, delta_number);
	delta_number = 0;
    }

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Check the change state.
    //
    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
#ifndef DEBUG
    default:
#endif
    case cstate_state_awaiting_development:
	change_fatal(cp, 0, i18n("bad patch send state"));

    case cstate_state_completed:
	//
	// Need to reconstruct the appropriate file histories.
	//
	trace(("project = \"%s\"\n", project_name_get(pp)->str_text));
	project_file_roll_forward
	(
	    pp,
	    (
		delta_date != NO_TIME_SET
	    ?
		delta_date
	    :
		change_completion_timestamp(cp)
	    ),
	    0
	);
	break;

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
	break;
    }

    // open the output
    os_become_orig();
    if (ascii_armor == content_encoding_unset)
	ascii_armor = content_encoding_base64;
    if (ascii_armor != content_encoding_none || !compress)
	ofp = output_file_text_open(output);
    else
	ofp = output_file_binary_open(output);
    output_fputs(ofp, "MIME-Version: 1.0\n");
    output_fputs(ofp, "Content-Type: application/aegis-patch\n");
    content_encoding_header(ofp, ascii_armor);
    s = project_name_get(pp);
    s2 = cstate_data->brief_description;
    output_fprintf
    (
	ofp,
	"Subject: %.*s - %.*s\n",
	len_printable(s, 40),
	s->str_text,
	len_printable(s2, 80),
	s2->str_text
    );
    output_fprintf
    (
	ofp,
	"Content-Name: %s.C%3.3ld.patch%s\n",
	project_name_get(pp)->str_text,
	change_number,
	(compress ? ".gz" : "")
    );
    output_fprintf
    (
	ofp,
	"Content-Disposition: attachment; filename=%s.C%3.3ld.patch%s\n",
	project_name_get(pp)->str_text,
	change_number,
	(compress ? ".gz" : "")
    );
    output_fprintf
    (
	ofp,
	"X-Aegis-Project-Name: %s\n",
	project_name_get(pp)->str_text
    );
    output_fprintf(ofp, "X-Aegis-Change-Number: %ld\n", cp->number);
    output_fputc(ofp, '\n');
    ofp = output_content_encoding(ofp, ascii_armor);
    if (compress)
	ofp = output_gzip(ofp);

    //
    // Add the change details to the archive.
    // This is done as a simple comment.
    //
    t1 = output_prefix(ofp, 0, "#\t");
    t2 = output_wrap_open(t1, 1, 70);
    os_become_undo();
    s = change_description_get(cp);
    os_become_orig();
    output_fputc(t2, '\n');
    output_put_str(t2, s);
    output_end_of_line(t2);
    output_fputc(t2, '\n');
    output_delete(t2);
    os_become_undo();

    if (use_meta_data)
    {
	cstate_ty       *change_set;

	//
	// Build the meta-data to write to the output.
	//
	change_set = (cstate_ty *)cstate_type.alloc();
	change_set->brief_description =
	    str_copy(cstate_data->brief_description);
	change_set->description = str_copy(cstate_data->description);
	change_set->cause = cstate_data->cause;
	change_set->test_exempt = cstate_data->test_exempt;
	change_set->test_baseline_exempt = cstate_data->test_baseline_exempt;
	change_set->regression_test_exempt =
	    cstate_data->regression_test_exempt;
	if (cstate_data->attribute)
	    change_set->attribute =
		attributes_list_copy(cstate_data->attribute);
	if (cstate_data->uuid)
	    change_set->uuid = str_copy(cstate_data->uuid);
	// architecture
	// copyright years

	for (j = 0;; ++j)
	{
	    fstate_src_ty   *src_data;
	    cstate_src_ty   **dst_data_p;
	    cstate_src_ty   *dst_data;
	    type_ty         *type_p;

	    src_data = change_file_nth(cp, j, view_path_first);
	    if (!src_data)
		break;
	    switch (src_data->usage)
	    {
	    case file_usage_build:
		switch (src_data->action)
		{
		case file_action_modify:
		    continue;

		case file_action_create:
		case file_action_remove:
		case file_action_insulate:
		case file_action_transparent:
		    break;
		}
		// fall through...

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		switch (src_data->action)
		{
		case file_action_create:
		case file_action_modify:
		    if (cstate_data->state < cstate_state_completed)
		    {
			s = change_file_path(cp, src_data->file_name);
			assert(s);
			if (s)
			{
			    os_become_orig();
			    src_data->executable = (boolean_ty)os_executable(s);
			    os_become_undo();
			    str_free(s);
			}
		    }
		    break;

		case file_action_remove:
		    break;

		case file_action_insulate:
		    continue;

		case file_action_transparent:
		    if
		    (
			src_data->about_to_be_created_by
		    ||
			src_data->about_to_be_copied_by
		    )
			continue;
		    break;
		}
		break;
	    }

	    //
	    // Aded the file to the list.
	    //
	    trace(("add \"%s\"\n", src_data->file_name->str_text));
	    if (!change_set->src)
		change_set->src =
		    (cstate_src_list_ty *)cstate_src_list_type.alloc();
	    dst_data_p =
		(cstate_src_ty **)
		cstate_src_list_type.list_parse(change_set->src, &type_p);
	    assert(type_p == &cstate_src_type);
	    dst_data = (cstate_src_ty *)cstate_src_type.alloc();
	    *dst_data_p = dst_data;
	    dst_data->file_name = str_copy(src_data->file_name);
	    dst_data->action = src_data->action;
	    dst_data->usage = src_data->usage;
	    dst_data->executable = src_data->executable;
	    if (src_data->move)
		dst_data->move = str_copy(src_data->move);
	    if (src_data->attribute)
		dst_data->attribute = attributes_list_copy(src_data->attribute);
	    if (src_data->uuid)
		dst_data->uuid = str_copy(src_data->uuid);
	}
	if (!change_set->src || !change_set->src->length)
	    change_fatal(cp, 0, i18n("bad send no files"));

	//
	// Now write the change set out.
	//
	os_become_orig();
	output_fputs(ofp, "# Aegis-Change-Set-Begin\n");
	t1 = output_prefix(ofp, 0, "# ");
	t2 = output_base64(t1, 1);
	t2 = output_gzip(t2);
	cstate_write(t2, change_set);
	output_delete(t2);
	output_fputs(ofp, "# Aegis-Change-Set-End\n#\n");
	os_become_undo();

	cstate_type.free(change_set);
    }

    //
    // We need a whole bunch of temporary files.
    //
    output_file_name = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(output_file_name);
    os_become_undo();
    dev_null = str_from_c("/dev/null");

    //
    // Add each of the relevant source files to the patch.
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *csrc;

	original_filename_unlink = 0;
	input_filename_unlink = 0;

	csrc = change_file_nth(cp, j, view_path_first);
	if (!csrc)
	    break;
	trace(("fn = \"%s\"\n", csrc->file_name->str_text));
	switch (csrc->usage)
	{
	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;

	case file_usage_build:
	    continue;
	}

	//
	// Find a source file.  Depending on the change state,
	// it could be in the development directory, or in the
	// baseline or in history.
	//
	// original_filename
	//      The oldest version of the file.
	// input_filename
	//      The youngest version of the file.
	// input_filename
	//      Where to write the output.
	//
	// These names are taken from the substitutions for
	// the diff_command.  It's historical.
	//
	ifp = 0;
	switch (cstate_data->state)
	{
	case cstate_state_awaiting_development:
	    assert(0);
	    continue;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	case cstate_state_being_integrated:
	    switch (csrc->action)
	    {
	    case file_action_create:
		original_filename = str_copy(dev_null);
		break;

	    case file_action_modify:
	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		original_filename =
		    project_file_version_path
		    (
			pp,
			csrc,
			&original_filename_unlink
		    );
		break;
	    }
	    switch (csrc->action)
	    {
	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		input_filename = change_file_path(cp, csrc->file_name);
		break;

	    case file_action_remove:
		input_filename = str_copy(dev_null);
		break;
	    }
	    break;

	case cstate_state_completed:
	    //
	    // Both the versions to be diffed come out
	    // of history.
	    //
	    switch (csrc->action)
	    {
		file_event_list_ty *felp;
		file_event_ty   *fep;
		fstate_src_ty   *old_src;

	    case file_action_create:
		original_filename = dev_null;
		input_filename =
		    project_file_version_path(pp, csrc, &input_filename_unlink);
		break;

	    case file_action_remove:
		felp = project_file_roll_forward_get(csrc->file_name);

		//
		// It's tempting to say
		//      assert(felp);
		// but file file may not yet exist at this point in
		// time, so there is no need (or ability) to create a
		// patch for it.
		//
		if (!felp)
		{
		    original_filename = str_copy(dev_null);
		    input_filename = str_copy(dev_null);
		    break;
		}

		//
		// It is tempting to say
		//	assert(felp->length >= 2);
		// except that a file which is created and removed in
		// the same branch, will result in only a remove record
		// in its parent branch when integrated.
		//
		assert(felp->length >= 1);

		if (felp->length < 2)
		{
		    original_filename = str_copy(dev_null);
		}
		else
		{
		    fep = &felp->item[felp->length - 2];
		    old_src =
			change_file_find
			(
			    fep->cp,
			    csrc->file_name,
			    view_path_first
			);
		    assert(old_src);
		    original_filename =
			project_file_version_path
			(
			    pp,
			    old_src,
			    &original_filename_unlink
			);
		}

		input_filename = str_copy(dev_null);
		break;

#ifndef DEBUG
	    default:
#endif
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		felp = project_file_roll_forward_get(csrc->file_name);

		//
		// It's tempting to say
		//      assert(felp);
		// but file file may not yet exist at this point in
		// time, so there is no need (or ability) to create a
		// patch for it.
		//
		if (!felp)
		{
		    original_filename = str_copy(dev_null);
		    input_filename = str_copy(dev_null);
		    break;
		}

		assert(felp->length >= 2);

		fep = &felp->item[felp->length - 2];
		old_src =
		    change_file_find(fep->cp, csrc->file_name, view_path_first);
		assert(old_src);
		original_filename =
		    project_file_version_path
		    (
			pp,
			old_src,
			&original_filename_unlink
		    );

		fep = &felp->item[felp->length - 1];
		old_src =
		    change_file_find(fep->cp, csrc->file_name, view_path_first);
		assert(old_src);
		input_filename =
		    project_file_version_path
		    (
			pp,
			old_src,
			&input_filename_unlink
		    );
		break;
	    }
	    break;
	}

	//
	// If they are both /dev/null don't bother with a patch.
	//
	trace(("original_filename = \"%s\"\n", original_filename->str_text));
	trace(("input_filename = \"%s\"\n", input_filename->str_text));
	if
	(
	    str_equal(original_filename, dev_null)
	&&
	    str_equal(input_filename, dev_null)
	)
	{
	    str_free(original_filename);
	    str_free(input_filename);
	    continue;
	}

	//
	// Generate the difference file.
	//
	assert(original_filename);
	assert(input_filename);
	change_run_patch_diff_command
	(
	    cp,
	    up,
	    original_filename,
	    input_filename,
	    output_file_name,
	    csrc->file_name
	);

	os_become_orig();
	if (original_filename_unlink)
	{
	    os_unlink_errok(original_filename);
	    str_free(original_filename);
	}
	if (input_filename_unlink)
	{
	    os_unlink_errok(input_filename);
	    str_free(input_filename);
	}
	os_become_undo();

	//
	// Read the diff into the patch output.
	//
	trace(("open \"%s\"\n", output_file_name->str_text));
	os_become_orig();
	ifp = input_file_open(output_file_name);
	input_file_unlink_on_close(ifp);
	if (input_length(ifp) != 0)
	{
	    output_fputs(ofp, "Index: ");
	    output_put_str(ofp, csrc->file_name);
	    output_fputc(ofp, '\n');
	    input_to_output(ifp, ofp);
	}
	input_delete(ifp);
	ifp = 0;
	os_become_undo();
	str_free(output_file_name);
    }

    //
    // Get rid of all the temporary files.
    //
    os_become_orig();
    os_unlink_errok(output_file_name);
    os_become_undo();
    str_free(output_file_name);
    str_free(dev_null);

    //
    // Mark the end of the patch.
    //
    os_become_orig();
    output_delete(ofp);
    os_become_undo();

    // clean up and go home
    change_free(cp);
    project_free(pp);
}
