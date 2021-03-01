//
//	aegis - project change supervisor
//	Copyright (C) 2002-2008 Peter Miller
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h> // assert
#include <common/gettime.h>
#include <common/now.h>
#include <common/nstring.h>
#include <common/progname.h>
#include <common/str_list.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/compres_algo.h>
#include <libaegis/file/event.h>
#include <libaegis/help.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/output/bzip2.h>
#include <libaegis/output/file.h>
#include <libaegis/output/gzip.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aedist/send.h>
#include <aetar/arglex3.h>
#include <aetar/output/tar.h>


#define NO_TIME_SET ((time_t)(-1))


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


void
tar_send(void)
{
    string_ty       *project_name;
    long            change_number;
    const char      *branch;
    int             grandparent;
    int             trunk;
    project_ty      *pp;
    change::pointer cp;
    user_ty::pointer up;
    cstate_ty       *cstate_data;
    string_ty       *output_filename;
    size_t          j;
    int             baseline;
    int             entire_source;
    int             include_build = -1;
    long            delta_number;
    time_t          delta_date;
    const char      *delta_name;

    branch = 0;
    change_number = 0;
    grandparent = 0;
    project_name = 0;
    trunk = 0;
    output_filename = 0;
    baseline = 0;
    entire_source = -1;
    compression_algorithm_t needs_compression = compression_algorithm_not_set;
    delta_date = NO_TIME_SET;
    delta_number = -1;
    delta_name = 0;
    nstring path_prefix;
    const char *compatibility = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_baseline:
	    if (baseline)
		duplicate_option(usage);
	    baseline = 1;
	    break;

	case arglex_token_entire_source:
	    if (entire_source > 0)
		duplicate_option(usage);
	    if (entire_source >= 0)
	    {
		mutually_exclusive_options
		(
		    arglex_token_entire_source,
		    arglex_token_entire_source_not,
		    usage
		);
	    }
	    entire_source = 1;
	    break;

	case arglex_token_entire_source_not:
	    if (entire_source == 0)
		duplicate_option(usage);
	    if (entire_source >= 0)
	    {
		mutually_exclusive_options
		(
		    arglex_token_entire_source,
		    arglex_token_entire_source_not,
		    usage
		);
	    }
	    entire_source = 0;
	    break;

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
	    if (output_filename)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_output, usage);
		// NOTREACHED

	    case arglex_token_stdio:
		output_filename = str_from_c("");
		break;

	    case arglex_token_string:
		output_filename = str_from_c(arglex_value.alv_string);
		break;
	    }
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

	case arglex_token_path_prefix_add:
	    if (!path_prefix.empty())
		duplicate_option(usage);
	    if (arglex() != arglex_token_string)
	    {
		option_needs_string(arglex_token_delta_date, usage);
		// NOTREACHED
	    }
	    path_prefix = arglex_value.alv_string;
	    break;

        case arglex_token_include_build:
            if (include_build > 0)
                duplicate_option(usage);
            if (include_build >= 0)
            {
                mutually_exclusive_options
                (
                    arglex_token_include_build,
                    arglex_token_include_build_not,
                    usage
                );
            }
            include_build = 1;
            break;

        case arglex_token_include_build_not:
            if (include_build == 0)
                duplicate_option(usage);
            if (include_build >= 0)
            {
                mutually_exclusive_options
                (
                    arglex_token_include_build,
                    arglex_token_include_build_not,
                    usage
                );
            }
            include_build = 0;
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
    if (change_number && baseline)
    {
	mutually_exclusive_options
	(
	    arglex_token_change,
	    arglex_token_baseline,
	    usage
	);
    }
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

    if (entire_source < 0)
	entire_source = 0;

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
    if (baseline)
	cp = change_copy(pp->change_get());
    else
    {
	if (!change_number)
	    change_number = up->default_change(pp);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
    }

    //
    // Check the change state.
    //
    cstate_data = cp->cstate_get();
    project_file_roll_forward historian;
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
#ifndef DEBUG
    default:
#endif
	change_fatal(cp, 0, i18n("bad send state"));

    case cstate_state_completed:
	//
	// Need to reconstruct the appropriate file histories.
	//
	historian.set
	(
	    pp,
	    (
		(delta_date != NO_TIME_SET)
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

    //
    // Figure what to do for compatibility.
    //
    bool use_bzip2 = true;
    if (compatibility)
    {
	//
	// The bzip compression algorithm was first available in
	// Peter's 4.21.D186, publicly in 4.22
	//
	use_bzip2 = (strverscmp(compatibility, "4.22") >= 0);

	//
	// Add new compatibility tests above this comment.
	//
    }

    //
    // Refine the compression to be used.
    //
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
    case compression_algorithm_unspecified:
	needs_compression =
	    (
		use_bzip2
	    ?
		compression_algorithm_bzip2
	    :
	       	compression_algorithm_gzip
	    );
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

    //
    // open the output
    //
    os_become_orig();
    output::pointer out_fp = output_file::binary_open(output_filename);
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
    case compression_algorithm_unspecified:
	assert(0);
	// fall through...

    case compression_algorithm_none:
	break;

    case compression_algorithm_gzip:
	out_fp = output_gzip::create(out_fp);
	break;

    case compression_algorithm_bzip2:
	out_fp = output_bzip2::create(out_fp);
	break;
    }
    output_tar *tar_p = new output_tar(out_fp);
    os_become_undo();

    //
    // Scan for files to be added to the output.
    //
    string_list_ty wl;
    for (j = 0;; ++j)
    {
	fstate_src_ty   *src_data;

	src_data = change_file_nth(cp, j, view_path_first);
	if (!src_data)
	    break;
	switch (src_data->action)
	{
	case file_action_create:
	case file_action_remove:
	    break;

	case file_action_insulate:
	case file_action_transparent:
	    continue;

	case file_action_modify:
	    switch (src_data->usage)
	    {
	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		break;

	    case file_usage_build:
                if (include_build <= 0)
                    continue;
                break;
	    }
	    break;
	}
	wl.push_back(src_data->file_name);
    }
    if (entire_source)
    {
	//
	// Actually, this list needs to be at the time of the delta.
	// (So, we keep almost all files, and toss them later.)
	//
	if (historian.is_set())
	{
	    nstring_list file_name_list;
	    historian.keys(file_name_list);
	    for (j = 0; j < file_name_list.size(); ++j)
	    {
		nstring file_name = file_name_list[j];
		assert(file_name);
		file_event *fep = historian.get_last(file_name);
		assert(fep);
		if (!fep)
		    continue;
		fstate_src_ty *src_data = fep->get_src();
		assert(src_data);
		if (!src_data)
		    continue;
		if
		(
		    attributes_list_find_boolean
		    (
			src_data->attribute,
			"entire-source-hide"
		    )
		)
		{
		    continue;
		}
		switch (src_data->usage)
		{
		case file_usage_build:
		    switch (src_data->action)
		    {
		    case file_action_modify:
		    case file_action_remove:
			continue;

		    case file_action_create:
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
			break;

		    case file_action_remove:
			continue;

		    case file_action_insulate:
		    case file_action_transparent:
			// can't happen
			assert(0);
			continue;
		    }
		    break;
		}
		wl.push_back_unique(src_data->file_name);
	    }
	}
	else
	{
	    for (j = 0;; ++j)
	    {
		fstate_src_ty   *src_data;

		src_data = pp->file_nth(j, view_path_extreme);
		if (!src_data)
		    break;
		if
		(
		    attributes_list_find_boolean
		    (
			src_data->attribute,
			"entire-source-hide"
		    )
		)
		{
		    continue;
		}
		wl.push_back_unique(src_data->file_name);
	    }
	}
    }
    if (!wl.nstrings)
	change_fatal(cp, 0, i18n("bad send no files"));

    //
    // sort the files by name
    //
    wl.sort();

    //
    // add each of the relevant source files to the archive
    //
    if (!path_prefix.empty() && path_prefix[path_prefix.size() - 1] != '/')
	path_prefix = path_prefix + "/";
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_ty	*filename;
	fstate_src_ty   *csrc = 0;
	long            len;
	string_ty       *abs_filename = 0;
	int             abs_filename_unlink = 0;

	filename = wl.string[j];

	switch (cstate_data->state)
	{
	    file_event  *fep;

	case cstate_state_awaiting_development:
	    assert(0);
	    continue;

	case cstate_state_being_developed:
	case cstate_state_awaiting_review:
	case cstate_state_being_reviewed:
	case cstate_state_awaiting_integration:
	case cstate_state_being_integrated:
	    //
	    // Get the input file.
	    //
	    csrc = change_file_find(cp, filename, view_path_first);
	    if (!csrc)
		csrc = project_file_find(pp, filename, view_path_simple);
	    switch (csrc->action)
	    {
	    case file_action_create:
	    case file_action_modify:
	    case file_action_transparent:
		break;

	    case file_action_remove:
	    case file_action_insulate:
		continue;
	    }
	    switch (csrc->usage)
	    {
	    case file_usage_build:
                if (include_build <= 0)
                    continue;
                break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		break;
	    }
	    if (csrc->deleted_by)
		continue;
	    if (csrc->about_to_be_created_by)
		continue;

	    abs_filename = change_file_path(cp, csrc->file_name);
	    if (!abs_filename)
	        abs_filename = project_file_path(pp, csrc->file_name);
	    abs_filename_unlink = 0;

	    //
	    // The executable field is only set by aeipass, so we need
	    // to go and look at the file itself for accurate information
	    // for changes which are not yet in the completed state.
	    //
	    os_become_orig();
	    csrc->executable = os_executable(abs_filename);
	    os_become_undo();
	    break;

	case cstate_state_completed:
	    //
	    // Extract the file from history.
	    //
	    fep = historian.get_last(filename);
	    if (!fep)
		continue;
	    csrc = fep->get_src();
	    assert(csrc);
	    if (!csrc)
		continue;
	    switch (csrc->action)
	    {
	    case file_action_create:
	    case file_action_modify:
	    case file_action_transparent:
		break;

	    case file_action_remove:
	    case file_action_insulate:
		continue;
	    }
	    switch (csrc->usage)
	    {
	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_test:
	    case file_usage_manual_test:
		break;

	    case file_usage_build:
                if (include_build <= 0)
                    continue;
                break;
	    }
	    abs_filename =
		project_file_version_path
		(
		    pp,
		    csrc,
		    &abs_filename_unlink
		);
	    break;
	}

	//
	// Put the whole file into the archive.
	//
	assert(csrc);
	switch (csrc->action)
	{
	case file_action_remove:
	case file_action_transparent:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    {
		os_become_orig();
		input ifp = input_file_open(abs_filename);
		assert(ifp.is_open());
		len = ifp->length();
		nstring tar_name = path_prefix + nstring(filename);
		output::pointer ofp =
                    tar_p->child(tar_name, len, csrc->executable);
		ofp << ifp;
		ifp.close();
		ofp.reset();
		os_become_undo();
	    }
	    break;
	}

	//
	// Free a bunch of strings.
	//
	if (abs_filename_unlink)
	{
	    os_become_orig();
	    os_unlink_errok(abs_filename);
	    os_become_undo();
	}
	str_free(abs_filename);
    }

    //
    // finish writing the tar archive
    //
    os_become_orig();
    delete tar_p;
    os_become_undo();

    //
    // clean up and go home
    //
    change_free(cp);
    project_free(pp);
}
