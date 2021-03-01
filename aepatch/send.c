/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate sends
 */

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <arglex3.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <error.h>
#include <gettime.h>
#include <help.h>
#include <input/file.h>
#include <option.h>
#include <os.h>
#include <output/conten_encod.h>
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
    /* Intentionally the C locale, not the user's locale */
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
    char            *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --send [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static string_ty *
change_description_get(change_ty *cp)
{
    cstate          cstate_data;

    cstate_data = change_cstate_get(cp);
    return cstate_data->description;
}


void
send(void)
{
    string_ty       *original_filename = 0;
    int             original_filename_unlink;
    string_ty       *input_filename = 0;
    int             input_filename_unlink;
    string_ty       *output_file_name;
    output_ty       *t1;
    output_ty       *t2;
    string_ty       *project_name;
    long            change_number;
    char            *branch;
    int             grandparent;
    int             trunk;
    output_ty       *ofp;
    input_ty        *ifp;
    project_ty      *pp;
    change_ty       *cp;
    user_ty         *up;
    cstate          cstate_data;
    string_ty       *output;
    string_ty       *s;
    string_ty       *s2;
    size_t          j;
    content_encoding_t ascii_armor;
    int             compress;
    string_ty       *dev_null;
    long            delta_number;
    time_t          delta_date;
    char            *delta_name;

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
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_change, usage);
	    /* fall throught... */

	case arglex_token_number:
	    if (change_number)
		duplicate_option_by_name(arglex_token_change, usage);
	    change_number = arglex_value.alv_number;
	    if (!change_number)
		change_number = MAGIC_ZERO;
	    else if (change_number < 0)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
	    }
	    break;

	case arglex_token_project:
	    if (arglex() != arglex_token_string)
		option_needs_name(arglex_token_project, usage);
	    if (project_name)
		duplicate_option_by_name(arglex_token_project, usage);
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

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
		/* NOTREACHED */

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
		/*NOTREACHED*/

	    case arglex_token_number:
		delta_number = arglex_value.alv_number;
		if (delta_number < 0)
		{
		    sub_context_ty *scp;

		    scp = sub_context_new();
		    sub_var_set_long(scp, "Number", delta_number);
		    fatal_intl(scp, i18n("delta $number out of range"));
		    /* NOTREACHED */
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
		/*NOTREACHED*/
	    }
	    delta_date = date_scan(arglex_value.alv_string);
	    if (delta_date == NO_TIME_SET)
		fatal_date_unknown(arglex_value.alv_string);
	    break;
	}
	arglex();
    }

    /*
     * reject illegal combinations of options
     */
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

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate the other branch
     */
    if (branch)
	pp = project_find_branch(pp, branch);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * it is an error if the delta does not exist
     */
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
	time_t          now;

	/*
	 * If the time is in the future, you could get a different
	 * answer for the same input at some point in the future.
	 *
	 * This is the "time safe" quality first described by
	 * Damon Poole <damon@ede.com>
	 */
	time(&now);
	if (delta_date > now)
	    project_error(pp, 0, i18n("date in the future"));

	/*
    	 * Now find the change number corresponding.
	 */
	change_number = project_history_change_by_timestamp(pp, delta_date);
    }
    if (delta_number >= 0)
    {
	/* does not return if no such delta number */
	change_number = project_history_change_by_delta(pp, delta_number);
	delta_number = 0;
    }

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * Check the change state.
     */
    cstate_data = change_cstate_get(cp);
    switch (cstate_data->state)
    {
    default:
	change_fatal(cp, 0, i18n("bad patch send state"));

    case cstate_state_completed:
	/*
	 * Need to reconstruct the appropriate file histories.
	 */
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

    /* open the output */
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
	"Content-Name: %s.C%3.3ld.patch\n",
	project_name_get(pp)->str_text,
	change_number
    );
    output_fprintf
    (
	ofp,
	"Content-Disposition: attachment; filename=%s.C%3.3ld.patch\n",
	project_name_get(pp)->str_text,
	change_number
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

    /*
     * Add the change details to the archive.
     * This is done as a simple comment.
     */
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

    /*
     * We need a whole bunch of temporary files.
     */
    output_file_name = os_edit_filename(0);
    os_become_orig();
    undo_unlink_errok(output_file_name);
    os_become_undo();
    dev_null = str_from_c("/dev/null");

    /*
     * Add each of the relevant source files to the patch.
     */
    for (j = 0;; ++j)
    {
	fstate_src      csrc;

	original_filename_unlink = 0;
	input_filename_unlink = 0;

	csrc = change_file_nth(cp, j);
	if (!csrc)
	    break;
	trace(("fn = \"%s\"\n", csrc->file_name->str_text));
	if (csrc->usage == file_usage_build)
	    continue;

	/*
	 * Find a source file.  Depending on the change state,
	 * it could be in the development directory, or in the
	 * baseline or in history.
	 *
	 * original_filename
	 *      The oldest version of the file.
	 * input_filename
	 *      The youngest version of the file.
	 * input_filename
	 *      Where to write the output.
	 *
	 * These names are taken from the substitutions for
	 * the diff_command.  It's historical.
	 */
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
	    if (csrc->action == file_action_create)
		original_filename = str_copy(dev_null);
	    else
	    {
		original_filename =
		    project_file_version_path
		    (
			pp,
			csrc,
			&original_filename_unlink
		    );
	    }
	    if (csrc->action != file_action_remove)
	    {
		input_filename = change_file_path(cp, csrc->file_name);
	    }
	    else
		input_filename = str_copy(dev_null);
	    break;

	case cstate_state_completed:
	    /*
	     * Both the versions to be diffed come out
	     * of history.
	     */
	    switch (csrc->action)
	    {
		file_event_list_ty *felp;
		file_event_ty  *fep;
		fstate_src      old_src;

	    case file_action_create:
		original_filename = dev_null;
		input_filename =
		    project_file_version_path(pp, csrc, &input_filename_unlink);
		break;

	    case file_action_remove:
		felp = project_file_roll_forward_get(csrc->file_name);
		assert(felp);
		assert(felp->length >= 2);

		fep = &felp->item[felp->length - 2];
		old_src = change_file_find(fep->cp, csrc->file_name);
		assert(old_src);
		original_filename =
		    project_file_version_path
		    (
			pp,
			old_src,
			&original_filename_unlink
		    );

		input_filename = str_copy(dev_null);
		break;

	    default:
		felp = project_file_roll_forward_get(csrc->file_name);
		assert(felp);
		assert(felp->length >= 2);

		fep = &felp->item[felp->length - 2];
		old_src = change_file_find(fep->cp, csrc->file_name);
		assert(old_src);
		original_filename =
		    project_file_version_path
		    (
			pp,
			old_src,
			&original_filename_unlink
		    );

		fep = &felp->item[felp->length - 1];
		old_src = change_file_find(fep->cp, csrc->file_name);
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

	/*
	 * Generate the difference file.
	 */
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

	/*
	 * Read the diff into the patch output.
	 */
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

    /*
     * Get rid of all the temporary files.
     */
    os_become_orig();
    os_unlink_errok(output_file_name);
    os_become_undo();
    str_free(output_file_name);
    str_free(dev_null);

    /*
     * Mark the end of the patch.
     */
    os_become_orig();
    output_delete(ofp);
    os_become_undo();

    /* clean up and go home */
    change_free(cp);
    project_free(pp);
}
