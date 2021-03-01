//
//	aegis - project change supervisor
//	Copyright (C) 2002-2004 Peter Miller;
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

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <error.h> // assert
#include <help.h>
#include <input/file.h>
#include <send.h>
#include <gettime.h>
#include <now.h>
#include <nstring.h>
#include <os.h>
#include <output/file.h>
#include <output/gzip.h>
#include <output/tar.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <project/history.h>
#include <str_list.h>
#include <sub.h>
#include <user.h>


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
    output_ty       *ofp;
    output_ty       *tar_p;
    input_ty        *ifp;
    project_ty      *pp;
    change_ty       *cp;
    user_ty         *up;
    cstate_ty       *cstate_data;
    string_ty       *output;
    size_t          j;
    int             baseline;
    int             entire_source;
    int             compress;
    long            delta_number;
    time_t          delta_date;
    const char      *delta_name;
    string_list_ty  wl;

    branch = 0;
    change_number = 0;
    grandparent = 0;
    project_name = 0;
    trunk = 0;
    output = 0;
    baseline = 0;
    entire_source = -1;
    compress = -1;
    delta_date = NO_TIME_SET;
    delta_number = -1;
    delta_name = 0;
    nstring path_prefix;
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
    if (baseline)
	cp = change_copy(project_change_get(pp));
    else
    {
	if (!change_number)
	    change_number = user_default_change(up);
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
    }

    //
    // Check the change state.
    //
    cstate_data = change_cstate_get(cp);
    project_file_roll_forward historian;
    switch (cstate_data->state)
    {
    default:
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
    // open the output
    //
    os_become_orig();
    ofp = output_file_binary_open(output);
    if (compress)
	ofp = output_gzip(ofp);
    tar_p = output_tar(ofp);
    os_become_undo();

    //
    // Scan for files to be added to the output.
    //
    string_list_constructor(&wl);
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
		continue;
	    }
	    break;
	}
	string_list_append(&wl, src_data->file_name);
    }
    if (entire_source)
    {
	//
	// Actually, this list needs to be at the time of the delta.
	// (So, we keep almost all files, and toss them later.)
	//
	for (j = 0;; ++j)
	{
	    fstate_src_ty   *src_data;

	    src_data = project_file_nth(pp, j, view_path_extreme);
	    if (!src_data)
		break;
	    string_list_append_unique(&wl, src_data->file_name);
	}
    }
    if (!wl.nstrings)
	change_fatal(cp, 0, i18n("bad send no files"));

    //
    // sort the files by name
    //
    string_list_sort(&wl);

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

	ifp = 0;
	switch (cstate_data->state)
	{
	    file_event_ty  *fep;

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
		continue;

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
	    csrc = change_file_find(fep->cp, filename, view_path_first);
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
		continue;
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
		ifp = input_file_open(abs_filename);
		assert(ifp);
		len = input_length(ifp);
		nstring tar_name = path_prefix + nstring(str_copy(filename));
		ofp =
		    output_tar_child
		    (
			tar_p,
			tar_name.get_ref(),
			len,
			csrc->executable
		    );
		input_to_output(ifp, ofp);
		input_delete(ifp);
		output_delete(ofp);
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
    string_list_destructor(&wl);

    //
    // finish writing the tar archive
    //
    os_become_orig();
    output_delete(tar_p);
    os_become_undo();

    //
    // clean up and go home
    //
    change_free(cp);
    project_free(pp);
}
