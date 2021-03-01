//
//	aegis - project change supervisor
//	Copyright (C) 2001-2007 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/gettime.h>
#include <common/now.h>
#include <common/progname.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/version_stmp.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/signedoffby.h>
#include <libaegis/compres_algo.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/help.h>
#include <libaegis/input/file.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/base64.h>
#include <libaegis/output/bzip2.h>
#include <libaegis/output/conten_encod.h>
#include <libaegis/output/file.h>
#include <libaegis/output/gzip.h>
#include <libaegis/output/prefix.h>
#include <libaegis/output/wrap.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aepatch/send.h>
#include <aepatch/arglex3.h>


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
change_description_get(change::pointer cp)
{
    cstate_ty       *cstate_data;

    cstate_data = cp->cstate_get();
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
    project_ty      *pp;
    change::pointer cp;
    user_ty::pointer up;
    cstate_ty       *cstate_data;
    string_ty       *output;
    string_ty       *s;
    string_ty       *s2;
    size_t          j;
    content_encoding_t ascii_armor;
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
    compression_algorithm_t needs_compression = compression_algorithm_not_set;
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
	    if (needs_compression != compression_algorithm_not_set)
	    {
		duplicate_option_by_name
	       	(
		    arglex_token_compression_algorithm,
		    usage
		);
	    }
	    needs_compression = compression_algorithm_unspecified;
	    break;

	case arglex_token_compress_not:
	    if (needs_compression != compression_algorithm_not_set)
	    {
		duplicate_option_by_name
	       	(
		    arglex_token_compression_algorithm,
		    usage
		);
	    }
	    needs_compression = compression_algorithm_none;
	    break;

	case arglex_token_compression_algorithm:
	    if (arglex() != arglex_token_string)
	    {
		option_needs_string(arglex_token_compression_algorithm, usage);
		// NOTREACHED
	    }
	    else
	    {
		compression_algorithm_t temp =
		    compression_algorithm_by_name(arglex_value.alv_string);

		//
		// We don't complain if the answer is going to be the same,
		// for compatibility with the old options.
		//
		if (temp == needs_compression)
		    break;

		switch (needs_compression)
		{
		case compression_algorithm_not_set:
		case compression_algorithm_unspecified:
		    needs_compression = temp;
		    break;

		case compression_algorithm_none:
		case compression_algorithm_gzip:
		case compression_algorithm_bzip2:
		    duplicate_option_by_name
		    (
			arglex_token_compression_algorithm,
			usage
		    );
		    // NOTREACHED
		}
	    }
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

	case arglex_token_signed_off_by:
	case arglex_token_signed_off_by_not:
	    option_signed_off_by_argument(usage);
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
    bool use_bzip2 = true;
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

	//
	// Use the bzip compression algorithm, introduced
	// in Peter's 4.21.D147, publicly in 4.22
	//
	use_bzip2 = (strverscmp(compatibility, "4.22") >= 0);
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate the other branch
    //
    if (branch)
	pp = pp->find_branch(branch);

    //
    // locate user data
    //
    up = user_ty::create();

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
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // If the user asked for one, append a Signed-off-by line to this
    // change's description.  (Since we don't write the cstate back out,
    // it is safe to change the change's description.)
    //
    if (option_signed_off_by_get(false))
	change_signed_off_by(cp, up);

    //
    // Check the change state.
    //
    cstate_data = cp->cstate_get();
    project_file_roll_forward historian;
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
	historian.set
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
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
	if (ascii_armor == content_encoding_none)
	{
	    needs_compression = compression_algorithm_none;
	    break;
	}
	// Fall through...

    case compression_algorithm_unspecified:
	if (use_bzip2)
	    needs_compression = compression_algorithm_bzip2;
	else
	    needs_compression = compression_algorithm_gzip;
	break;

    case compression_algorithm_none:
	break;

    case compression_algorithm_gzip:
	use_bzip2 = false;
	break;

    case compression_algorithm_bzip2:
	use_bzip2 = true;
	break;
    }
    if
    (
	ascii_armor == content_encoding_none
    &&
	needs_compression != compression_algorithm_none
    )
	ofp = output_file_binary_open(output);
    else
	ofp = output_file_text_open(output);
    ofp->fputs("MIME-Version: 1.0\n");
    ofp->fputs("Content-Type: application/aegis-patch\n");
    content_encoding_header(ofp, ascii_armor);
    s = project_name_get(pp);
    s2 = cstate_data->brief_description;
    ofp->fprintf
    (
	"Subject: %.*s - %.*s\n",
	len_printable(s, 40),
	s->str_text,
	len_printable(s2, 80),
	s2->str_text
    );
    ofp->fprintf
    (
	"Content-Name: %s.C%3.3ld.patch%s\n",
	project_name_get(pp)->str_text,
	change_number,
	compression_algorithm_extension(needs_compression)
    );
    ofp->fprintf
    (
	"Content-Disposition: attachment; filename=%s.C%3.3ld.patch%s\n",
	project_name_get(pp)->str_text,
	change_number,
	compression_algorithm_extension(needs_compression)
    );
    ofp->fprintf
    (
	"X-Aegis-Project-Name: %s\n",
	project_name_get(pp)->str_text
    );
    ofp->fprintf("X-Aegis-Change-Number: %ld\n", cp->number);
    ofp->fputc('\n');
    ofp = output_content_encoding(ofp, ascii_armor);
    switch (needs_compression)
    {
    case compression_algorithm_none:
	break;

    case compression_algorithm_not_set:
    case compression_algorithm_unspecified:
	assert(0);
	// Fall through...

    case compression_algorithm_bzip2:
	ofp = new output_bzip2(ofp, true);
	break;

    case compression_algorithm_gzip:
	ofp = new output_gzip(ofp, true);
	break;
    }

    //
    // Add the change details to the archive.
    // This is done as a simple comment.
    //
    t1 = new output_prefix_ty(ofp, false, "#\t");
    t2 = output_wrap_open(t1, 1, 70);
    os_become_undo();
    s = change_description_get(cp);
    os_become_orig();
    t2->fputc('\n');
    t2->fputs(s);
    t2->end_of_line();
    t2->fputc('\n');
    delete t2;
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
	    meta_type *type_p = 0;

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
			    src_data->executable = os_executable(s);
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
	    // Add the file to the list.
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
	ofp->fputs("# Aegis-Change-Set-Begin\n");
	t1 = new output_prefix_ty(ofp, false, "# ");
	t2 = new output_base64_ty(t1, true);
	if (use_bzip2)
	    t2 = new output_bzip2(t2, true);
	else
	    t2 = new output_gzip(t2, true);
	cstate_write(t2, change_set);
	delete t2;
	ofp->fputs("# Aegis-Change-Set-End\n#\n");
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
	    case file_action_create:
		original_filename = dev_null;
		input_filename =
		    project_file_version_path(pp, csrc, &input_filename_unlink);
		break;

	    case file_action_remove:
                {
                    file_event_list::pointer felp =
                        historian.get(csrc->file_name);

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
                    //     assert(felp->length >= 2);
                    // except that a file which is created and removed in
                    // the same branch, will result in only a remove record
                    // in its parent branch when integrated.
                    //
                    assert(!felp->empty());

                    if (felp->size() < 2)
                    {
                        original_filename = str_copy(dev_null);
                    }
                    else
                    {
                        file_event *fep = felp->get(felp->size() - 2);
                        fstate_src_ty *old_src = fep->get_src();
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
                }
		break;

#ifndef DEBUG
	    default:
#endif
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
                {
                    file_event_list::pointer felp =
                        historian.get(csrc->file_name);

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

                    assert(felp->size() >= 2);

                    file_event *fep = felp->get(felp->size() - 2);
                    fstate_src_ty *old_src = fep->get_src();
                    assert(old_src);
                    original_filename =
                        project_file_version_path
                        (
                            pp,
                            old_src,
                            &original_filename_unlink
                        );

                    fep = felp->back();
                    old_src = fep->get_src();
                    assert(old_src);
                    input_filename =
                        project_file_version_path
                        (
                            pp,
                            old_src,
                            &input_filename_unlink
                        );
                }
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

	//
	// Read the diff into the patch output.
	//
	trace(("open \"%s\"\n", output_file_name->str_text));
	input ifp = input_file_open(output_file_name, true);
	assert(ifp.is_open());
	if (ifp->length() != 0)
	{
	    ofp->fputs("Index: ");
	    ofp->fputs(csrc->file_name);
	    ofp->fputc('\n');
	    *ofp << ifp;
	}
	ifp.close();
	os_become_undo();
	str_free(output_file_name);
    }

    //
    // Get rid of all the temporary files.
    //
    os_become_orig();
    os_unlink_errok(output_file_name);
    str_free(output_file_name);
    str_free(dev_null);

    //
    // Mark the end of the patch.
    //
    delete ofp;
    os_become_undo();

    // clean up and go home
    change_free(cp);
    project_free(pp);
}
