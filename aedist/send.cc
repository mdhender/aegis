//
//	aegis - project change supervisor
//	Copyright (C) 1999-2005 Peter Miller;
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
#include <ac/stdlib.h>
#include <ac/string.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/functor/attribu_list.h>
#include <change/signedoffby.h>
#include <change.h>
#include <error.h>	// for assert
#include <gettime.h>
#include <help.h>
#include <input/file.h>
#include <mem.h>
#include <now.h>
#include <option.h>
#include <os.h>
#include <output/conten_encod.h>
#include <output/cpio.h>
#include <output/file.h>
#include <output/gzip.h>
#include <output/indent.h>
#include <project/file.h>
#include <project/file/roll_forward.h>
#include <project/invento_walk.h>
#include <project.h>
#include <project/history.h>
#include <send.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <usage.h>
#include <user.h>


#define NO_TIME_SET ((time_t)(-1))


static int
have_it_already(cstate_ty *change_set, fstate_src_ty *src_data)
{
    size_t          j;
    cstate_src_ty   *dst_data;

    if (!change_set->src)
	return 0;
    for (j = 0; j < change_set->src->length; ++j)
    {
	dst_data = change_set->src->list[j];
	if (str_equal(dst_data->file_name, src_data->file_name))
	    return 1;
    }
    return 0;
}


static void
one_more_src(cstate_ty *change_set, fstate_src_ty *src_data, int use_attr)
{
    cstate_src_ty   **dst_data_p;
    cstate_src_ty   *dst_data;
    type_ty         *type_p;

    trace(("add \"%s\"\n", src_data->file_name->str_text));
    if (!change_set->src)
	change_set->src = (cstate_src_list_ty *)cstate_src_list_type.alloc();
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
    if (use_attr)
    {
	if (src_data->attribute)
	    dst_data->attribute = attributes_list_copy(src_data->attribute);
	if (src_data->uuid)
	    dst_data->uuid = str_copy(src_data->uuid);
    }
}


static int
cmp(const void *va, const void *vb)
{
    cstate_src_ty   *a;
    cstate_src_ty   *b;

    a = *(cstate_src_ty **)va;
    b = *(cstate_src_ty **)vb;
    return strcmp(a->file_name->str_text, b->file_name->str_text);
}


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


void
send_main(void)
{
    int             use_attributes;
    int		    use_patch;
    int		    use_change_number;
    int             use_config;
    int             use_rename_patch;
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
    cstate_ty       *change_set;
    time_t          when;
    size_t          j;
    int             description_header;
    int             baseline;
    int             entire_source;
    content_encoding_t ascii_armor;
    int             needs_compression;
    string_ty       *dev_null;
    string_ty       *diff_output_filename;
    long            delta_number;
    time_t          delta_date;
    const char      *delta_name;
    const char      *compatibility;

    arglex();
    compatibility = 0;
    branch = 0;
    change_number = 0;
    grandparent = 0;
    project_name = 0;
    trunk = 0;
    output = 0;
    description_header = -1;
    baseline = 0;
    entire_source = -1;
    ascii_armor = content_encoding_unset;
    needs_compression = -1;
    delta_date = NO_TIME_SET;
    delta_number = -1;
    delta_name = 0;
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

	case arglex_token_description_header:
	    if (description_header == 1)
		duplicate_option(usage);
	    else if (description_header >= 0)
	    {
	        head_desc_yuck:
		mutually_exclusive_options
		(
		    arglex_token_description_header,
		    arglex_token_description_header_not,
		    usage
		);
	    }
	    description_header = 1;
	    break;

	case arglex_token_description_header_not:
	    if (description_header == 0)
		duplicate_option(usage);
	    else if (description_header >= 0)
		goto head_desc_yuck;
	    description_header = 0;
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
	    if (needs_compression > 0)
		duplicate_option(usage);
	    else if (needs_compression >= 0)
	    {
	        compress_yuck:
		mutually_exclusive_options
		(
		    arglex_token_compress,
		    arglex_token_compress_not,
		    usage
		);
	    }
	    needs_compression = 1;
	    break;

	case arglex_token_compress_not:
	    if (needs_compression == 0)
		duplicate_option(usage);
	    else if (needs_compression >= 0)
		goto compress_yuck;
	    needs_compression = 0;
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

	case arglex_token_patch:
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_charstar
		(
		    scp,
		    "Name1",
		    arglex_token_name(arglex_token_patch)
		);
		sub_var_set_charstar
		(
		    scp,
		    "Name2",
		    arglex_token_name(arglex_token_compatibility)
		);
		error_intl
		(
		    scp,
		    i18n("warning: $name1 obsolete, use $name2 option")
		);
		sub_context_delete(scp);

		if (compatibility)
		    duplicate_option_by_name(arglex_token_compatibility, usage);
		compatibility = "99999999";
	    }
	    break;

	case arglex_token_patch_not:
	    {
		sub_context_ty  *scp;

		scp = sub_context_new();
		sub_var_set_charstar
		(
		    scp,
		    "Name1",
		    arglex_token_name(arglex_token_patch)
		);
		sub_var_set_format
		(
		    scp,
		    "Name2",
		    "%s=4.6",
		    arglex_token_name(arglex_token_compatibility)
		);
		error_intl
		(
		    scp,
		    i18n("warning: $name1 obsolete, use $name2 option")
		);
		sub_context_delete(scp);

		if (compatibility)
		    duplicate_option_by_name(arglex_token_compatibility, usage);
		compatibility = "4.6";
	    }
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
    if (entire_source < 0)
	entire_source = baseline;

    //
    // Translate the compatibility version number into a set of
    // capabilities.
    //
    use_patch = 1;
    use_change_number = 1;
    use_config = 1;
    use_attributes = 1; // implies UUIDs as well
    use_rename_patch = 1;
    if (compatibility)
    {
	//
	// FIXME: should we check that it actually looks like a version
	// string?
	//

	//
	// Patches were introduced in Peter's 4.6.D022,
	// publicly in 4.7
	//
	use_patch = (strverscmp(compatibility, "4.7") >= 0);

	//
	// Change numbers were introduced in Peter's 4.9.D035,
	// publicly in 4.10
	//
	use_change_number = (strverscmp(compatibility, "4.10") >= 0);

	//
	// The config file usage was introduced in Peter's 4.16.D024,
	// publicly in 4.17
	//
	use_config = (strverscmp(compatibility, "4.17") >= 0);

	//
        // The file attributes and change attributes were added to
	// aedist -send in Peter's 4.16.D054, publicly in 4.17
	//
        // The change UUIDs were added to
	// aedist -send in Peter's 4.16.D089, publicly in 4.17
	//
	use_attributes = use_config;

        //
        // The patch for renamed files were added to aedist -send in
        // Peter's 4.18.D004, publicly in 4.19
        //
        use_rename_patch = (strverscmp(compatibility, "4.19") >= 0);
    }
    if (entire_source)
    {
	use_patch = 0;
	use_rename_patch = 0;
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
    when = now();
    if (delta_date != NO_TIME_SET)
    {
	//
	// If the time is in the future, you could get a different
	// answer for the same input at some point in the future.
	//
	// This is the "time safe" quality first described by
	// Damon Poole <damon@ede.com>
	//
	if (delta_date > when)
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
    // If the use asked for one, append a Signed-off-by line to this
    // change's description.  (Since we don't write the cstate back out,
    // it is safe to change the change's description.)
    //
    if (option_signed_off_by_get(false))
	change_signed_off_by(cp, up);

    //
    // Check the change state.
    //
    cstate_data = change_cstate_get(cp);
    project_file_roll_forward historian;
    switch (cstate_data->state)
    {
    case cstate_state_awaiting_development:
#ifndef DEBUG
    default:
#endif
	change_fatal(cp, 0, i18n("bad send state"));
	// NOTREACHED

    case cstate_state_being_integrated:
    case cstate_state_awaiting_integration:
    case cstate_state_being_reviewed:
    case cstate_state_awaiting_review:
    case cstate_state_being_developed:
    case cstate_state_completed:
	//
	// Need to reconstruct the appropriate file histories even for
	// outstanding changes because some file may be renamed and we
	// need to extract the old file from the baseline.
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
    }

    // open the output
    os_become_orig();
    if (ascii_armor == content_encoding_unset)
	ascii_armor = content_encoding_base64;
    if (ascii_armor != content_encoding_none || !needs_compression)
	ofp = output_file_text_open(output);
    else
	ofp = output_file_binary_open(output);
    ofp->fputs("MIME-Version: 1.0\n");
    ofp->fputs("Content-Type: application/aegis-change-set\n");
    content_encoding_header(ofp, ascii_armor);
    string_ty *s1 = project_name_get(pp);
    string_ty *s2 = cstate_data->brief_description;
    if (entire_source)
	s2 = project_description_get(pp);
    ofp->fprintf
    (
	"Subject: %.*s - %.*s\n",
	len_printable(s1, 40),
	s1->str_text,
	len_printable(s2, 80),
	s2->str_text
    );
    if (change_number && !entire_source)
    {
	ofp->fprintf
	(
	    "Content-Name: %s.C%3.3ld.ae\n",
	    project_name_get(pp)->str_text,
	    change_number
	);
	ofp->fprintf
	(
	    "Content-Disposition: attachment; filename=%s.C%3.3ld.ae\n",
	    project_name_get(pp)->str_text,
	    change_number
	);
    }
    else
    {
	ofp->fprintf("Content-Name: %s.ae\n", project_name_get(pp)->str_text);
	ofp->fprintf
	(
	    "Content-Disposition: attachment; filename=%s.ae\n",
	    project_name_get(pp)->str_text
	);
    }
    ofp->fputc('\n');
    ofp = output_content_encoding(ofp, ascii_armor);
    if (needs_compression)
	ofp = new output_gzip_ty(ofp, true);
    output_cpio_ty *cpio_p = new output_cpio_ty(ofp);

    //
    // Add the project name to the archive.
    //
    nstring childs_name = "etc/project-name";
    ofp = cpio_p->child(childs_name, -1);
    ofp->fprintf("%s\n", project_name_get(pp)->str_text);
    delete ofp;

    //
    // Add the change number to the archive.
    //
    if (use_change_number)
    {
	childs_name = "etc/change-number";
	ofp = cpio_p->child(childs_name, -1);
	ofp->fprintf("%ld\n", change_number);
	delete ofp;
    }
    os_become_undo();

    //
    // Add the change details to the archive.
    //
    change_set = (cstate_ty *)cstate_type.alloc();
    if (entire_source)
    {
	change_set->brief_description = str_copy(project_description_get(pp));
    }
    else
    {
	change_set->brief_description =
	    str_copy(cstate_data->brief_description);
    }
    if (description_header)
    {
	string_ty       *warning;

	warning = 0;
	if
	(
	    entire_source
	?
	    cstate_data->state < cstate_state_being_integrated
	:
	    cstate_data->state != cstate_state_completed
	)
	{
	    warning =
		str_format
		(
		    "Warning: the original change was in the '%s' state\n",
		    cstate_state_ename(cstate_data->state)
		);
	}
	if (cstate_data->state > cstate_state_being_developed)
	{
	    cstate_history_list_ty *hlp;

	    hlp = cstate_data->history;
	    assert(hlp);
	    assert(hlp->length > 0);
	    assert(hlp->list);
	    if (hlp && hlp->length > 0 && hlp->list)
	    {
		cstate_history_ty *hp;

		hp = hlp->list[hlp->length - 1];
		assert(hp);
		when = hp->when;
	    }
	}
	change_set->description =
	    str_format
	    (
		"From: %s\nDate: %.24s\n%s\n%s",
		user_email_address(up)->str_text,
		ctime(&when),
		(warning ? warning->str_text : ""),
		cstate_data->description->str_text
	    );
	if (warning)
	    str_free(warning);
    }
    else if (entire_source)
	change_set->description = str_copy(project_description_get(pp));
    else
	change_set->description = str_copy(cstate_data->description);
    change_set->cause = cstate_data->cause;
    change_set->test_exempt = cstate_data->test_exempt;
    change_set->test_baseline_exempt = cstate_data->test_baseline_exempt;
    change_set->regression_test_exempt = cstate_data->regression_test_exempt;
    if (use_attributes)
    {
	change_set->attribute =
	    (
		cstate_data->attribute
	    ?
		attributes_list_copy(cstate_data->attribute)
	    :
		(attributes_list_ty *)attributes_list_type.alloc()
	    );
	change_functor_attribute_list result(change_set->attribute);
	if (change_was_a_branch(cp))
	{
	    //
            // For branches, add all of the constituent change sets'
            // UUIDs.  That way, if you resynch by grabbing a whole
            // branch as one change set, you still grab all of the
            // constituent set UUIDs.
	    //
	    project_inventory_walk(pp, result);
	}
	if (entire_source)
	{
	    //
            // If they said --entire-source, add all of the accumulated
            // change set UUIDs.  That way, if you resynch by grabbing
            // a whole project as one change set, you still grab all of
            // the constituent change set UUIDs.
	    //
	    time_t limit = change_completion_timestamp(cp);
	    project_inventory_walk(pp, result, limit);
	}
	if (change_set->attribute->length == 0)
	{
	    attributes_list_type.free(change_set->attribute);
	    change_set->attribute = 0;
	}
    }
    if (use_attributes && cstate_data->uuid)
	change_set->uuid = str_copy(cstate_data->uuid);
    // architecture
    // copyright years

    //
    // Scan for files to be added to the output.
    //
    for (j = 0;; ++j)
    {
	fstate_src_ty   *src_data;

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
		    string_ty *s = change_file_path(cp, src_data->file_name);
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
	if (!use_config && src_data->usage == file_usage_config)
	    src_data->usage = file_usage_source;
	one_more_src(change_set, src_data, use_attributes);
    }
    if (entire_source)
    {
	if (historian.is_set())
	{
	    nstring_list file_name_list;
	    historian.keys(file_name_list);
	    for (j = 0; j < file_name_list.size(); ++j)
	    {
		nstring file_name = file_name_list[j];
		assert(file_name.length());
		file_event_ty *fep = historian.get_last(file_name);
		assert(fep);
		if (!fep)
		    continue;
		assert(fep->src);
		switch (fep->src->usage)
		{
		case file_usage_build:
		    switch (fep->src->action)
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
		    switch (fep->src->action)
		    {
		    case file_action_create:
		    case file_action_modify:
		    case file_action_remove:
			break;

		    case file_action_insulate:
		    case file_action_transparent:
			// can't happen
			assert(0);
			continue;
		    }
		    break;
		}
		if (!have_it_already(change_set, fep->src))
		{
		    if (!use_config && fep->src->usage == file_usage_config)
			fep->src->usage = file_usage_source;
		    one_more_src(change_set, fep->src, use_attributes);
		}
	    }
	}
	else
	{
	    for (j = 0;; ++j)
	    {
		fstate_src_ty   *src_data;

		src_data = project_file_nth(pp, j, view_path_simple);
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
		    case file_action_remove:
			break;

		    case file_action_insulate:
		    case file_action_transparent:
			// can't happen
			assert(0);
			continue;
		    }
		    break;
		}
		if (!have_it_already(change_set, src_data))
		{
		    if (!use_config && src_data->usage == file_usage_config)
			src_data->usage = file_usage_source;
		    one_more_src(change_set, src_data, use_attributes);
		}
	    }
	}
    }
    if (!change_set->src || !change_set->src->length)
	change_fatal(cp, 0, i18n("bad send no files"));

    //
    // sort the files by name
    //
    qsort
    (
	change_set->src->list,
	change_set->src->length,
	sizeof(change_set->src->list[0]),
	cmp
    );

    os_become_orig();
    childs_name = "etc/change-set";
    ofp = cpio_p->child(childs_name, -1);
    ofp = new output_indent_ty(ofp, true);
    cstate_write(ofp, change_set);
    delete ofp;
    os_become_undo();

    //
    // We need a whole bunch of temporary files.
    //
    diff_output_filename = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(diff_output_filename);
    os_become_undo();
    dev_null = str_from_c("/dev/null");

    //
    // add each of the relevant source files to the archive
    //
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *csrc;
	long            len;
	string_ty       *original_filename = 0;
	string_ty       *input_filename = 0;
	int             original_filename_unlink = 0;
	int             input_filename_unlink = 0;

	csrc = change_set->src->list[j];
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
	switch (csrc->action)
	{
	case file_action_remove:
	    continue;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
	trace(("file name = \"%s\"\n", csrc->file_name->str_text));

	//
	// Find a source file.  Depending on the change state,
	// it could be in the development directory, or in the
	// baseline or in history.
	//
	// original_filename
	//      The oldest version of the file.
	// input_filename
	//      The youngest version of the file.
	// diff_output_filename
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
	    //
	    // Get the orginal file.
	    //
	    switch (csrc->action)
	    {
	    case file_action_create:
                if (use_rename_patch && csrc->move)
                {
                    file_event_list_ty  *orig_felp;
                    file_event_ty       *orig_fep;
                    fstate_src_ty       *orig_src;

                    orig_felp = historian.get(csrc->move);

                    //
                    // It's tempting to say
                    //     assert(felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
                    //
                    assert(!orig_felp || orig_felp->length >= 1);
                    if (!orig_felp)
                    {
                        original_filename = str_copy(dev_null);
                        break;
                    }

                    orig_fep = &orig_felp->item[orig_felp->length - 1];
                    orig_src =
                        change_file_find
                        (
                            orig_fep->cp,
                            csrc->move,
                            view_path_first
                        );
                    assert(orig_src);
                    original_filename =
                        project_file_version_path
                        (
                            pp,
                            orig_src,
                            &original_filename_unlink
                        );
                    fstate_src_type.free(orig_src);
                }
                else
                    original_filename = str_copy(dev_null);
		break;

	    case file_action_modify:
	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		original_filename = project_file_path(pp, csrc->file_name);
		break;
	    }
	    assert(original_filename);

	    //
	    // Get the input file.
	    //
	    switch (csrc->action)
	    {
	    case file_action_remove:
		input_filename = str_copy(dev_null);
		break;

	    case file_action_transparent:
		// FIXME: this is wrong, need version from grandparent
		// fall through...

	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
#ifndef DEBUG
	    default:
#endif
		input_filename = change_file_path(cp, csrc->file_name);
		if (!input_filename)
		    input_filename = project_file_path(pp, csrc->file_name);
		break;
	    }
	    assert(input_filename);
	    break;

	case cstate_state_completed:
	    //
	    // Both the versions to be diffed come out
	    // of history.
	    //
            switch (csrc->action)
	    {
		file_event_list_ty *felp;
		file_event_ty  *fep;
		fstate_src_ty  *old_src;

	    case file_action_create:
		felp = historian.get(csrc->file_name);

		//
		// It's tempting to say
		//	assert(felp);
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

		assert(felp->length >= 1);

		//
		// Get the orginal file.  We handle the creation half
		// of a file rename.
		//
                if (use_rename_patch && csrc->move)
                {
                    file_event_list_ty  *orig_felp;
                    file_event_ty       *orig_fep;
                    fstate_src_ty       *orig_src;

                    orig_felp = historian.get(csrc->move);

                    //
                    // It's tempting to say
                    //     assert(orig_felp);
                    // but file file may not yet exist at this point in
                    // time, so there is no need (or ability) to create a
                    // patch for it.
		    //
		    // It is also tempting to think that for every
		    // remove there must be a corresponding create.
		    // However if a file is created an removed on a
		    // branch, and the branch is ended, the parent
		    // branch only sees the remove.
		    //
		    // This means you can have a removed file with a
		    // history length of exactly one.
                    //
		    assert(!orig_felp || orig_felp->length >= 1);
                    if (!orig_felp || orig_felp->length < 2)
                    {
                        original_filename = str_copy(dev_null);
                    }
		    else
		    {
			orig_fep = &orig_felp->item[orig_felp->length - 2];
			assert(orig_fep);
			orig_src =
			    change_file_find
			    (
				orig_fep->cp,
				csrc->move,
				view_path_first
			    );
			assert(orig_src);
			original_filename =
			    project_file_version_path
			    (
				pp,
				orig_src,
				&original_filename_unlink
			    );
			fstate_src_type.free(orig_src);
		    }
                }
                else
                    original_filename = str_copy(dev_null);

		//
		// Get the input file.
		//
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
		assert(original_filename);
		assert(input_filename);
		break;

	    case file_action_remove:
                //
                // We ignore the remove half or a file rename.
                //
                if (use_rename_patch && csrc->move)
                {
                    input_filename = str_copy(dev_null);
                    original_filename = str_copy(dev_null);
                    break;
                }

		felp = historian.get(csrc->file_name);

		//
		// It's tempting to say
		//	assert(felp);
		// but file file may not yet exist at this point in
		// time, so there is no need (or ability) to create a
		// patch for it.
		//
		// It is also tempting to say
		//	assert(felp->length >= 2);
		// except that a file which is created and removed in
		// the same branch, will result in only a remove record
		// in its parent branch when integrated.
		//
		assert(!felp || felp->length >= 1);
		if (!felp || felp->length < 2)
		{
		    original_filename = str_copy(dev_null);
		}
		else
		{
		    //
		    // Get the orginal file.
		    //
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

		//
		// Get the input file.
		//
		input_filename = str_copy(dev_null);
		break;

	    case file_action_modify:
		felp = historian.get(csrc->file_name);

		//
		// It's tempting to say
		//	assert(felp);
		// but file file may not yet exist at this point in
		// time, so there is no need (or ability) to create a
		// patch for it.
		//
		assert(!felp || felp->length >= 1);
		if (!felp)
		{
		    original_filename = str_copy(dev_null);
		    input_filename = str_copy(dev_null);
		    break;
		}

		//
		// Get the orginal file.
		//
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

		//
		// Get the input file.
		//
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

	    case file_action_insulate:
		// this is supposed to be impossible
		trace(("insulate = \"%s\"\n", csrc->file_name->str_text));
		assert(0);
		original_filename = str_copy(dev_null);
		input_filename = str_copy(dev_null);
		break;

	    case file_action_transparent:
		// no file content appears in the output
		trace(("transparent = \"%s\"\n", csrc->file_name->str_text));
		original_filename = str_copy(dev_null);
		input_filename = str_copy(dev_null);
		break;
	    }
	    assert(original_filename);
	    assert(input_filename);
	    break;
	}

	//
	// If they are both /dev/null don't bother with a patch.
	//
	assert(original_filename);
	assert(input_filename);
	if
	(
	    !str_equal(original_filename, dev_null)
	||
	    !str_equal(input_filename, dev_null)
	)
	{
	    //
	    // Put a patch into the archive
	    // for modified files.
	    //
	    // We don't bother with a patch for created files, because
	    // we simply include the whole source in the next section.
	    //
	    bool is_a_rename = false;
	    switch (csrc->action)
	    {
	    case file_action_remove:
	    case file_action_transparent:
		break;

            case file_action_create:
                if (!use_rename_patch || !csrc->move)
                    break;
		is_a_rename = true;
                // fall through

            case file_action_modify:
	    case file_action_insulate:
		if (entire_source)
		    break;
		if (!use_patch)
		    break;

		//
		// Generate the difference file.
		//
		assert(original_filename);
		trace(("original_filename = \"%s\"\n",
		    original_filename->str_text));
		assert(input_filename);
		trace(("input_filename = \"%s\"\n",
		    input_filename->str_text));
		assert(diff_output_filename);
		trace(("diff_output_filename = \"%s\"\n",
		    diff_output_filename->str_text));
		change_run_patch_diff_command
		(
		    cp,
		    up,
		    original_filename,
		    input_filename,
		    diff_output_filename,
		    csrc->file_name
		);

		//
		// Read the diff into the archive.
		//
		os_become_orig();
		ifp = input_file_open(diff_output_filename);
		assert(ifp);
		input_file_unlink_on_close(ifp);
		len = input_length(ifp);
                assert(len >= 0);
		if (len > 0 || is_a_rename)
		{
		    childs_name =
			nstring::format("patch/%s", csrc->file_name->str_text);
                    ofp = cpio_p->child(childs_name, len);
		    input_to_output(ifp, ofp);
		    delete ofp;
		}
		input_delete(ifp);
		os_become_undo();

                // It's tempting to say:
		//
		// str_free(diff_output_filename);
		//
		// but this must really be freed once: out of the loop.
		break;
	    }
	}

	//
	// Put the whole file into the archive,
	// for creates and modifies.
	//
	// Even though a patch is preferable, sometimes the
	// destination has never heard of the file, so we include
	// the whole file just in case.
	//
	switch (csrc->action)
	{
	case file_action_remove:
	case file_action_transparent:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    os_become_orig();
	    ifp = input_file_open(input_filename);
	    assert(ifp);
	    len = input_length(ifp);
	    childs_name =
		nstring::format("src/%s", csrc->file_name->str_text);
	    ofp = cpio_p->child(childs_name, len);
	    input_to_output(ifp, ofp);
	    input_delete(ifp);
	    delete ofp;
	    os_become_undo();
	    break;
	}

	//
	// Free a bunch of strings.
	//
	os_become_orig();
	if (original_filename_unlink)
	    os_unlink_errok(original_filename);
	if (input_filename_unlink)
	    os_unlink_errok(input_filename);
	os_become_undo();
        str_free(original_filename);
        str_free(input_filename);
    }
    cstate_type.free(change_set);

    //
    // Get rid of all the temporary files.
    //
    os_become_orig();
    assert(diff_output_filename);
    os_unlink_errok(diff_output_filename);
    os_become_undo();
    str_free(diff_output_filename);
    str_free(dev_null);

    // finish writing the cpio archive
    os_become_orig();
    delete cpio_p;
    os_become_undo();

    // clean up and go home
    change_free(cp);
    project_free(pp);
}
