//
//	aegis - project change supervisor
//	Copyright (C) 2005-2008 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert
#include <common/trace.h>
#include <common/version_stmp.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/change/functor/attribu_list.h>
#include <libaegis/change/identifier.h>
#include <libaegis/change/signedoffby.h>
#include <libaegis/compres_algo.h>
#include <libaegis/cstate.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/fstate.h>
#include <libaegis/help.h>
#include <libaegis/input/file.h>
#include <libaegis/option.h>
#include <libaegis/os.h>
#include <libaegis/output/bzip2.h>
#include <libaegis/output/conten_encod.h>
#include <libaegis/output/file.h>
#include <libaegis/output/gzip.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project/history.h>
#include <libaegis/project/invento_walk.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

#include <aerevml/usage.h>
#include <aerevml/arglex3.h>
#include <aerevml/output/revml_encode.h>


#define NO_TIME_SET ((time_t)(-1))


static bool
have_it_already(fstate_src_list_ty *file_list, fstate_src_ty *src_data)
{
    assert(file_list);
    assert(src_data);
    for (size_t j = 0; j < file_list->length; ++j)
    {
	fstate_src_ty *dst_data = file_list->list[j];
	assert(dst_data);
	if (str_equal(dst_data->file_name, src_data->file_name))
	    return true;
    }
    return false;
}


static void
one_more_src(fstate_src_list_ty *file_list, fstate_src_ty *src_data)
{
    trace(("add \"%s\"\n", src_data->file_name->str_text));
    if (src_data->action == file_action_remove && src_data->move)
    {
        // Ignore the remove half of a move because we will use the
        // "move" action for the create half.
	return;
    }
    assert(file_list);
    meta_type *type_p = 0;
    fstate_src_ty **dst_data_p =
	(fstate_src_ty **)fstate_src_list_type.list_parse(file_list, &type_p);
    assert(type_p == &fstate_src_type);
    fstate_src_ty *dst_data = (fstate_src_ty *)fstate_src_type.alloc();
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


static void
one_more_src_unique(fstate_src_list_ty *file_list, fstate_src_ty *src_data)
{
    if (!have_it_already(file_list, src_data))
	one_more_src(file_list, src_data);
}


static int
cmp(const void *va, const void *vb)
{
    const fstate_src_ty *a = *(const fstate_src_ty **)va;
    const fstate_src_ty *b = *(const fstate_src_ty **)vb;
    return strcasecmp(a->file_name->str_text, b->file_name->str_text);
}


static int
len_printable(string_ty *s, int len_max)
{
    if (!s)
	return 0;
    const char *cp = 0;
    // Intentionally the C locale, not the user's locale
    for (cp = s->str_text; *cp && isprint((unsigned char)*cp); ++cp)
	;
    int result = (cp - s->str_text);
    if (result > len_max)
	result = len_max;
    return result;
}


static void
output_attribute(output::pointer ofp, const nstring &name, const nstring &value)
{
    ofp->fputs("<attribute><name>");
    ofp->fputs(name.html_quote());
    ofp->fputs("</name>\n<value>");
    ofp->fputs(value.html_quote());
    ofp->fputs("</value></attribute>\n");
}


static void
output_attribute_extn(output::pointer ofp, const nstring &name,
    const nstring &value)
{
    output_attribute(ofp, "X-Aegis-" + name, value);
}


static void
output_attribute_extn_bool(output::pointer ofp, const nstring &name, bool value)
{
    output_attribute_extn(ofp, name, boolean_ename(value));
}


static void
output_attribute_user(output::pointer ofp, const nstring &name,
    const nstring &value)
{
    output_attribute(ofp, "User-" + name, value);
}


static bool
is_x_attr(const nstring &name)
{
    return
	(
	    (name[0] == 'x' || name[0] == 'X')
	&&
    	    name[1] == '-'
	&&
    	    strchr(name.c_str() + 2, '-')
	);
}


static void
output_attribute_list(output::pointer ofp, attributes_list_ty *alp)
{
    if (!alp)
	return;
    for (size_t j = 0; j < alp->length; ++j)
    {
	attributes_ty *ap = alp->list[j];
	assert(ap);
	assert(ap->name);
	nstring name(ap->name);
	assert(ap->value);
	nstring value(ap->value);
	if (is_x_attr(name))
	    output_attribute(ofp, name, value);
	else
	    output_attribute_user(ofp, nstring(ap->name), nstring(ap->value));
    }
}


static void
emit_time(output::pointer op, const char *name, time_t when)
{
    // From the RevML DTD:
    // ISO-8601 format in GMT/UCT0
    // 2000-12-31 23:59:59Z
    struct tm *when_tm = gmtime(&when);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S Z", when_tm);
    op->fprintf("<%s>%s</%s>\n", name, buffer, name);
}


class aerevml_bad_state:
    public change_functor
{
public:
    aerevml_bad_state() :
	change_functor(true)
    {
    }

    void
    operator()(change::pointer cp)
    {
	change_fatal(cp, 0, i18n("bad patch send state"));
    }
};

static aerevml_bad_state barf_adev;


static void
element(output::pointer ofp, const char *name, const nstring &value)
{
    ofp->fputc('<');
    ofp->fputs(name);
    ofp->fputc('>');
    ofp->fputs(value.html_quote());
    ofp->fputs("</");
    ofp->fputs(name);
    ofp->fputs(">\n");
}


static void
element(output::pointer ofp, const char *name, string_ty *value)
{
    element(ofp, name, nstring(value));
}


void
revml_send(void)
{
    change_identifier cid;
    const char *compatibility = 0;
    nstring output_filename;
    int description_header = -1;
    int entire_source = -1;
    content_encoding_t ascii_armor = content_encoding_unset;
    compression_algorithm_t needs_compression = compression_algorithm_not_set;
    int mime_header = -1;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_baseline:
	case arglex_token_branch:
	case arglex_token_change:
	case arglex_token_delta:
	case arglex_token_delta_date:
	case arglex_token_delta_from_change:
	case arglex_token_grandparent:
	case arglex_token_number:
	case arglex_token_project:
	case arglex_token_trunk:
	    cid.command_line_parse(usage);
	    continue;

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

	case arglex_token_output:
	    if (!output_filename.empty())
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_output, usage);
		// NOTREACHED

	    case arglex_token_stdio:
		output_filename = "-";
		break;

	    case arglex_token_string:
		output_filename = arglex_value.alv_string;
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

	case arglex_token_mime_header:
	    if (mime_header > 0)
		duplicate_option(usage);
	    else if (mime_header >= 0)
	    {
	        mime_header_yuck:
		mutually_exclusive_options
		(
		    arglex_token_mime_header,
		    arglex_token_mime_header_not,
		    usage
		);
	    }
	    mime_header = 1;
	    break;

	case arglex_token_mime_header_not:
	    if (mime_header == 0)
		duplicate_option(usage);
	    else if (mime_header >= 0)
		goto mime_header_yuck;
	    mime_header = 0;
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
    cid.command_line_check(usage);

    if (entire_source < 0)
	entire_source = cid.get_baseline();
    bool use_patch = true;
    if (entire_source)
	use_patch = false;
    bool use_bzip2 = true;

    if (compatibility)
    {
	// The bzip2 output was introduced in Peter's 4.21.D184,
	// and publicly in the 4.22 release.
	use_bzip2 = (strverscmp(compatibility, "4.22") >= 0);
    }

    //
    // If the user asked for one, append a Signed-off-by line to this
    // change's description.  (Since we don't write the cstate back out,
    // it is safe to change the change's description.)
    //
    if (option_signed_off_by_get(false))
	change_signed_off_by(cid.get_cp(), cid.get_up());

    //
    // Open the output
    //
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
    if (mime_header < 0)
    {
	//
	// Decide whether or not to include a MIME header
	// (usually useful when called by aeget).
	//
	mime_header = (ascii_armor != content_encoding_none);
    }
    os_become_orig();
    output::pointer ofp;
    if
    (
	ascii_armor == content_encoding_none
    &&
	needs_compression != compression_algorithm_none
    )
	ofp = output_file::binary_open(output_filename);
    else
	ofp = output_file::text_open(output_filename);
    assert(ofp);
    os_become_undo();
    if (mime_header)
    {
	ofp->fputs("MIME-Version: 1.0\n");
	ofp->fputs("Content-Type: application/revml\n");
	content_encoding_header(ofp, ascii_armor);
	string_ty *s = project_name_get(cid.get_pp());
	string_ty *s2 = change_brief_description_get(cid.get_cp());
	if (entire_source)
	    s2 = project_description_get(cid.get_pp());
	ofp->fprintf
	(
	    "Subject: %.*s - %.*s\n",
	    len_printable(s, 40),
	    s->str_text,
	    len_printable(s2, 80),
	    s2->str_text
	);
	if (!cid.get_baseline() && !entire_source)
	{
	    ofp->fprintf
	    (
		"Content-Name: %s.C%3.3ld.revml\n",
		project_name_get(cid.get_pp())->str_text,
		cid.get_change_number()
	    );
	    ofp->fprintf
	    (
		"Content-Disposition: attachment; "
		"filename=%s.C%3.3ld.revml\n",
		project_name_get(cid.get_pp())->str_text,
		cid.get_change_number()
	    );
	}
	else
	{
	    ofp->fprintf
	    (
		"Content-Name: %s.revml\n",
		project_name_get(cid.get_pp())->str_text
	    );
	    ofp->fprintf
	    (
		"Content-Disposition: attachment; filename=%s.revml\n",
		project_name_get(cid.get_pp())->str_text
	    );
	}
	ofp->fputc('\n');
    }
    ofp = output_content_encoding(ofp, ascii_armor);
    switch (needs_compression)
    {
    case compression_algorithm_not_set:
	assert(0);
	break;

    case compression_algorithm_none:
	break;

    case compression_algorithm_unspecified:
	assert(0);
	// Fall through...

    case compression_algorithm_gzip:
	ofp = output_gzip::create(ofp);
	break;

    case compression_algorithm_bzip2:
	ofp = output_bzip2::create(ofp);
	break;
    }

    //
    // Emit the doctype and initial wrapper element.
    // (We'll need to track the version carefully.)
    //
    ofp->fputs("<?xml version=\"1.0\"?>\n");
    nstring REVML_VERSION = "0.35";
    ofp->fputs("<!DOCTYPE revml PUBLIC \"-//-//DTD REVML ");
    ofp->fputs(REVML_VERSION);
    ofp->fputs("//EN\" \"revml.dtd\">\n<revml version=\"");
    ofp->fputs(REVML_VERSION);
    ofp->fputs("\">\n");

    //
    // The DTD is quite strict about the order of elements.
    //
    emit_time(ofp, "time", change_completion_timestamp(cid.get_cp()));
    element(ofp, "rep_type", "Aegis");

    //
    // From the RevML DTD:
    //
    // "The version number, platform, etc. for the repository.  This may
    // be needed so that import utilities can figure out what tags mean
    // what when a particular repository version changes.
    //
    // "This is often the output of 'p4 info' or 'cvs -v' + cvs
    // environment settings and the cvs -l command."
    //
    element(ofp, "rep_desc", version_stamp());

    //
    // Now the <comment> element.
    //
    cstate_ty *cstate_data = cid.get_cp()->cstate_get();
    nstring desc;
    if (description_header)
    {
	nstring warning;
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
		nstring::format
		(
		    "Warning: the original change was in the '%s' state\n",
		    cstate_state_ename(cstate_data->state)
		);
	}
	time_t when = change_completion_timestamp(cid.get_cp());
	desc =
	    nstring::format
	    (
		"From: %s\nDate: %.24s\n%s\n%s",
		cid.get_up()->get_email_address().c_str(),
		ctime(&when),
		warning.c_str(),
		cstate_data->description->str_text
	    );
    }
    else if (entire_source)
    {
	desc = nstring(project_description_get(cid.get_pp()));
    }
    else
    {
	desc = nstring(cstate_data->description);
    }
    element(ofp, "comment", desc);

    //
    // From the RevML DTD:
    //
    // "The root of the tree that was extracted to RevML.  This is
    // usually the source file spec up to (but not including) the first
    // component that does not contain a wildcard.
    //
    // "This comes before branches because it's needed to modify
    // branches, specifically the p4_branch_spec's View field.  That's a
    // theoretical issue as implemented, but it's nice to have things in
    // the safest, most mnemonic order, I think."
    //
    element(ofp, "rev_root", project_name_get(cid.get_pp()));

    //
    // Add the <attribute> elements.
    //
    output_attribute_extn
    (
	ofp,
	"change-number",
	nstring::format("%ld", magic_zero_decode(cid.get_change_number()))
    );
    output_attribute_extn
    (
	ofp,
	"brief-description",
	(
	    entire_source
	?
	    nstring(project_description_get(cid.get_pp()))
	:
	    nstring(cstate_data->brief_description)
	)
    );
    output_attribute_extn(ofp, "description", desc);
    output_attribute_extn(ofp, "cause", change_cause_ename(cstate_data->cause));
    output_attribute_extn_bool(ofp, "test-exempt", cstate_data->test_exempt);
    output_attribute_extn_bool
    (
	ofp,
	"test-baseline-exempt",
	cstate_data->test_baseline_exempt
    );
    output_attribute_extn_bool
    (
	ofp,
	"regression-test-exempt",
	cstate_data->regression_test_exempt
    );
    output_attribute_list(ofp, cstate_data->attribute);
    if (!cstate_data->attribute)
    {
	cstate_data->attribute =
	    (attributes_list_ty *)attributes_list_type.alloc();
    }
    change_functor_attribute_list result(false, cstate_data->attribute);
    if (change_was_a_branch(cid.get_cp()))
    {
	//
	// For branches, add all of the constituent change sets'
	// UUIDs.  That way, if you resynch by grabbing a whole
	// branch as one change set, you still grab all of the
	// constituent change set UUIDs.
	//
	project_inventory_walk(cid.get_pp(), result);
    }
    if (entire_source)
    {
	//
	// If they said --entire-source, add all of the accumulated
	// change set UUIDs.  That way, if you resynch by grabbing
	// a whole project as one change set, you still grab all of
	// the constituent change set UUIDs.
	//
	time_t limit = change_completion_timestamp(cid.get_cp());
	project_inventory_walk(cid.get_pp(), result, limit);
    }
    for (size_t an = 0; an < cstate_data->attribute->length; ++an)
    {
	attributes_ty *ap = cstate_data->attribute->list[an];
	output_attribute_user(ofp, nstring(ap->name), nstring(ap->value));
    }
    if (cstate_data->uuid)
    {
	output_attribute_extn(ofp, "uuid", nstring(cstate_data->uuid));
    }
    // architecture
    // copyright years

    //
    // Scan for files to be added to the output.
    //
    fstate_src_list_ty *file_list =
	(fstate_src_list_ty *)fstate_src_list_type.alloc();
    for (size_t j = 0;; ++j)
    {
	fstate_src_ty *src_data =
	    change_file_nth(cid.get_cp(), j, view_path_first);
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
		    string_ty *s =
			change_file_path(cid.get_cp(), src_data->file_name);
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
	one_more_src(file_list, src_data);
    }
    if (entire_source)
    {
	nstring_list file_name_list;
	cid.get_project_file_names(file_name_list);
	for (size_t j = 0; j < file_name_list.size(); ++j)
	{
	    fstate_src_ty *src_data = cid.get_project_file(file_name_list[j]);
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
	    one_more_src_unique(file_list, src_data);
	}
    }
    if (file_list->length == 0)
	change_fatal(cid.get_cp(), 0, i18n("bad send no files"));

    //
    // sort the files by name
    //
    qsort
    (
	file_list->list,
	file_list->length,
	sizeof(file_list->list[0]),
	cmp
    );

#if 0
    //
    // This was in an old version of the RevML spec.
    // It's gone from the 0.35 version.
    // It was supposed to allow the display of a progress meter.
    //
    element(ofp, "file_count", nstring::format("%ld", (long)file_list->length));
#endif

    //
    // We need a whole bunch of temporary files.
    //
    nstring diff_output_filename = nstring(os_edit_filename(0));
    os_become_orig();
    undo_unlink_errok(diff_output_filename);
    os_become_undo();
    nstring dev_null("/dev/null");

    //
    // Add each of the relevant source files to the archive
    // as <rev> elements.
    //
    for (size_t m = 0; m < file_list->length; ++m)
    {
	//
	// From the RevML DTD:
	//
        // "Each <rev> must have a unique identifier, usually the name
        // and revision number, though the precise format depends on
        // the source repository.  The ID should be derived from the
        // metadata in a repeatable fashion."
	//
	// This raises some questions:
	// 1. Unique for each file, or
	// 2. Unique for each change set, or
	// 3. both?
	//
	fstate_src_ty *csrc = file_list->list[m];
	trace(("file_name = \"%s\"\n", csrc->file_name->str_text));
	if (csrc->uuid)
	    ofp->fprintf("<rev id=\"%s\">\n", csrc->uuid->str_text);
	else
	{
	    nstring s(change_version_get(cid.get_cp()));
	    ofp->fprintf("<rev id=\"%s\">\n", s.c_str());
	}

	//
	// Emit the <name> elemrnt.
	//
	element
	(
	    ofp,
	    "name",
	    (
		csrc->action == file_action_create && csrc->move
	    ?
		csrc->move
	    :
		csrc->file_name
	    )
	);

	//
	// Emit the <source_name> element.
	//
	// Why is this mandatory is a mystery.
	//
	element
	(
	    ofp,
	    "source_name",
	    (
		csrc->action == file_action_create && csrc->move
	    ?
		csrc->move
	    :
		csrc->file_name
	    )
	);

	//
	// Emit the <source_filebranch_id> element.
	//
        // There is no comment in the RevML DTD explaining the use of
        // the element, or the range of legal values.
	//
	element(ofp, "source_filebranch_id", "no idea what this means");

	//
	// Emit the <source_repo_id> element.
	//
        // There is no comment in the RevML DTD explaining the use of
        // the element, or the range of legal values.
	//
	element(ofp, "source_repo_id", "no idea what this means");

	//
	// Find a source file.  Depending on the change state,
	// it could be in the development directory, or in the
	// baseline or in history.
	//
	// original
	//      The oldest version of the file.
	// input
	//      The youngest version of the file.
	// diff_output_filename
	//      Where to write the output.
	//
	// These names are taken from the substitutions for
	// the diff_command.  It's historical.
	//
	file_revision original(dev_null, false);
	file_revision input_rev(dev_null, false);
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
	    if (!entire_source)
	    {
		switch (csrc->action)
		{
		case file_action_create:
		    if (csrc->move)
		    {
			project_file_roll_forward *hp = cid.get_historian();
			file_event_list::pointer orig_felp =
                            hp->get(csrc->move);

			//
			// It's tempting to say
			//     assert(felp);
			// but file file may not yet exist at this point in
			// time, so there is no need (or ability) to create a
			// patch for it.
			//
			assert(!orig_felp || !orig_felp->empty());
			if (!orig_felp)
			    break;

			file_event *orig_fep = orig_felp->back();
			assert(orig_fep);
			int temp_unlink = 0;
			nstring temp =
			    nstring
			    (
				project_file_version_path
				(
				    cid.get_pp(),
				    orig_fep->get_src(),
				    &temp_unlink
				)
			    );
			original = file_revision(temp, temp_unlink);
		    }
		    break;

		case file_action_modify:
		case file_action_remove:
		case file_action_insulate:
		case file_action_transparent:
#ifndef DEBUG
		default:
#endif
		    original =
			file_revision
			(
			    nstring
			    (
				project_file_path(cid.get_pp(), csrc->file_name)
			    ),
			    false
			);
		    break;
		}
	    }

	    //
	    // Get the input file.
	    //
	    switch (csrc->action)
	    {
	    case file_action_remove:
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
		{
		    string_ty *s =
			change_file_path(cid.get_cp(), csrc->file_name);
		    if (!s)
			s = project_file_path(cid.get_pp(), csrc->file_name);
		    assert(s);
		    input_rev = file_revision(nstring(s), false);
		}
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
		{
		    project_file_roll_forward *hp = cid.get_historian();
		    file_event_list::pointer felp = hp->get(csrc);

		    //
		    // It's tempting to say
		    //     assert(felp);
		    // but file file may not yet exist at this point in
		    // time, so there is no need (or ability) to create a
		    // patch for it.
		    //
		    if (!felp)
		    {
			original = file_revision(dev_null, false);
			break;
		    }

		    assert(!felp->empty());

		    //
                    // Get the orginal file.  We handle the creation
                    // half of a file rename.
		    //
		    if (csrc->move)
		    {
			file_event_list::pointer orig_felp =
                            hp->get(csrc->move);

			//
			// It's tempting to say
			//     assert(orig_felp);
                        // but file file may not yet exist at this point
                        // in time, so there is no need (or ability) to
                        // create a patch for it.
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
			assert(!orig_felp || !orig_felp->empty());
			if (!orig_felp || orig_felp->size() < 2)
			{
			    original = file_revision(dev_null, false);
			}
			else
			{
			    file_event *orig_fep =
				orig_felp->get(orig_felp->size() - 2);
			    assert(orig_fep);
			    assert(orig_fep->get_src());
			    int path_unlink = 0;
			    nstring path =
				nstring
				(
				    project_file_version_path
				    (
					cid.get_pp(),
					orig_fep->get_src(),
					&path_unlink
				    )
				);
			    original = file_revision(path, path_unlink);
			}
		    }
		    else
			original = file_revision(dev_null, false);

		    //
		    // Get the input file.
		    //
		    file_event *fep = felp->back();
		    assert(fep->get_src());
		    int path_unlink = 0;
		    nstring path =
			nstring
			(
			    project_file_version_path
			    (
				cid.get_pp(),
				fep->get_src(),
				&path_unlink
			    )
			);
		    input_rev = file_revision(path, path_unlink);
		}
		break;

	    case file_action_remove:
		{
		    //
		    // We ignore the remove half of a file rename.
		    //
		    if (csrc->move)
		    {
			input_rev = file_revision(dev_null, false);
			original = file_revision(dev_null, false);
			break;
		    }

		    project_file_roll_forward *hp = cid.get_historian();
		    file_event_list::pointer felp = hp->get(csrc);

		    //
		    // It's tempting to say
		    //     assert(felp);
		    // but file file may not yet exist at this point in
		    // time, so there is no need (or ability) to create a
		    // patch for it.
		    //
		    // It is also tempting to say
		    //     assert(felp->length >= 2);
		    // except that a file which is created and removed in
		    // the same branch, will result in only a remove record
		    // in its parent branch when integrated.
		    //
		    assert(!felp || !felp->empty());
		    if (!felp || felp->size() < 2)
		    {
			original = file_revision(dev_null, false);
		    }
		    else
		    {
			//
			// Get the orginal file.
			//
			file_event *fep = felp->get(felp->size() - 2);
			assert(fep);
			assert(fep->get_src());
			int path_unlink = 0;
			nstring path =
			    nstring
			    (
				project_file_version_path
				(
				    cid.get_pp(),
				    fep->get_src(),
				    &path_unlink
				)
			    );
			original = file_revision(path, path_unlink);
		    }

		    //
		    // Get the input file.
		    //
		    input_rev = file_revision(dev_null, false);
		}
		break;

	    case file_action_modify:
		{
		    project_file_roll_forward *hp = cid.get_historian();
		    file_event_list::pointer felp = hp->get(csrc);

		    //
		    // It's tempting to say
		    //     assert(felp);
		    // but file file may not yet exist at this point in
		    // time, so there is no need (or ability) to create a
		    // patch for it.
		    //
		    assert(!felp || !felp->empty());
		    if (!felp)
		    {
			original = file_revision(dev_null, false);
			input_rev = file_revision(dev_null, false);
			break;
		    }

		    //
		    // Get the orginal file.
		    //
		    if (felp->size() < 2)
		    {
			original = file_revision(dev_null, false);
		    }
		    else
		    {
			file_event *fep = felp->get(felp->size() - 2);
			assert(fep);
			assert(fep->get_src());
			int path_unlink = 0;
			nstring path =
			    nstring
			    (
				project_file_version_path
				(
				    cid.get_pp(),
				    fep->get_src(),
				    &path_unlink
				)
			    );
			original = file_revision(path, path_unlink);
		    }

		    //
		    // Get the input file.
		    //
		    file_event *fep = felp->back();
		    assert(fep);
		    assert(fep->get_src());
		    int path_unlink = 0;
		    nstring path =
			nstring
			(
			    project_file_version_path
			    (
				cid.get_pp(),
				fep->get_src(),
				&path_unlink
			    )
			);
		    input_rev = file_revision(path, path_unlink);
		}
		break;

	    case file_action_insulate:
		// this is supposed to be impossible
		trace(("insulate = \"%s\"\n", csrc->file_name->str_text));
		assert(0);
		original = file_revision(dev_null, false);
		input_rev = file_revision(dev_null, false);
		break;

	    case file_action_transparent:
		// no file content appears in the output
		trace(("transparent = \"%s\"\n", csrc->file_name->str_text));
		original = file_revision(dev_null, false);
		input_rev = file_revision(dev_null, false);
		break;
	    }
	    break;
	}

	//
	// Emit the <action> element.
	//
	switch (csrc->action)
	{
	case file_action_create:
	    if (csrc->move)
	    {
		element(ofp, "action", "rename");
		element(ofp, "move", csrc->file_name);
	    }
	    else
		element(ofp, "action", "add");
	    break;

#ifndef DEBUG
	default:
#endif
	case file_action_transparent:
	case file_action_insulate:
	case file_action_modify:
	    element(ofp, "action", "edit");
	    break;

	case file_action_remove:
	    element(ofp, "action", "delete");
	    break;
	}

	//
	// Emit the optional <TYPE> element.
	//
	// Now that we have the input file, we can see what type it is,
	// if the file has no "Content-Type" attribute.
	//
	attributes_ty *ap =
	    attributes_list_find(csrc->attribute, "Content-Type");
	if (!ap || !ap->value || !ap->value->str_length)
	{
	    os_become_orig();
	    nstring content_type = os_magic_file(input_rev.get_path());
	    os_become_undo();
	    assert(!content_type.empty());
	    element(ofp, "type", content_type);
	}
	else
	{
	    element(ofp, "type", ap->value);
	}

	//
	// Emit the <REV_ID> element.
	//
        // Why is this mandatory?  And why is there a REV ID attribute
        // and also a REV_ID element.
	//
	// From the RevML DTD:
        // "A small integer indicating the revision number of a file
        // within the branch."
        //
	element(ofp, "rev_id", "0");

	//
	// Emit the <SOURCE_REV_ID> element.
	//
        // There is no comment in the RevML DTD explaining the use of
        // the element, or the range of legal values, or why it is
        // mandatory.
	//
	element(ofp, "source_rev_id", "no idea what this means");

	//
	// Emit the <ATTRIBUTE> elements.
	//
	output_attribute_extn(ofp, "usage", file_usage_ename(csrc->usage));
	output_attribute_extn(ofp, "action", file_action_ename(csrc->action));
        // FIXME: need to look at the change file if the change isn't
        // completed yet, because this attribute is set at aede.
	output_attribute_extn_bool(ofp, "executable", csrc->executable);
	if (!csrc->attribute)
	{
	    csrc->attribute =
		(attributes_list_ty *)attributes_list_type.alloc();
	}
	for (size_t j = 0; j < csrc->attribute->length; ++j)
	{
	    ap = csrc->attribute->list[j];
	    output_attribute_extn(ofp, nstring(ap->name), nstring(ap->value));
	}
	if (csrc->uuid)
	    output_attribute_extn(ofp, "uuid", nstring(csrc->uuid));

	//
	// Emit the <CONTENT> or <DELTA> elements.
	//
	switch (csrc->action)
	{
	case file_action_remove:
	    break;

	case file_action_insulate:
	case file_action_transparent:
	    assert(0);
	    // Fall through...

	case file_action_modify:
            // FIXME: need to send content for binary files, even when
            // we are just editing.
	    if (!entire_source && input_rev.get_path() != dev_null)
	    {
		//
		// Run the patch diff command to form the output.
		//
		change_run_patch_diff_command
		(
		    cid.get_cp(),
		    cid.get_up(),
		    original.get_path().get_ref(),
		    input_rev.get_path().get_ref(),
		    diff_output_filename.get_ref(),
		    csrc->file_name
		);

		//
		// Read the diff into the archive.
		//
		os_become_orig();
		input ifp = input_file_open(diff_output_filename, true);
		assert(ifp.is_open());
		if (ifp->length() != 0)
		{
                    // No newline here, or it will add a bogus blank
                    // line to the start of the delta.
		    ofp->fprintf("<delta type=\"diff-u\" encoding=\"none\">");
		    output::pointer ofp2 = output_revml_encode::create(ofp);
		    ofp2 << ifp;
		    ofp2.reset();
		    ofp->fputs("</delta>\n");
		}
		ifp.close();
		os_become_undo();
		break;
	    }
	    // fall through...

#ifndef DEBUG
	default:
#endif
	case file_action_create:
	    //
	    // FIXME: need to base64 encode binary files.
            //
            // Maybe we need to use the same trick as the history put
            // command, and use several encodings and send the smallest.
	    //
	    os_become_orig();
            // No newline here, or it will add a bogus blank line to the
            // start of the content.
	    ofp->fputs("<content encoding=\"none\">");
	    input ifp = input_file_open(input_rev.get_path().get_ref());
	    assert(ifp.is_open());
	    output::pointer ofp2 = output_revml_encode::create(ofp);
	    ofp2 << ifp;
	    ofp2.reset();
	    ifp.close();
	    ofp->fputs("</content>\n");
	    os_become_undo();
	    break;
	}

	//
	// Emit the optional <digest> element.
	//
        // FIXME: we are supposed to have a <digest> here, and it
        // probably contains an md5sum of the diff text we just emitted,
        // but it isn't described well enough to emit anything.  (Before
        // or after the base64 encoding?)

	//
	// End the open <rev> element.
	//
	ofp->fputs("</rev>\n");
    }

    // Do not
    //     fstate_src_type.free(file_list);
    // because this will free everything twice.
    file_list = 0;

    //
    // Get rid of all the temporary files.
    //
    os_become_orig();
    os_unlink_errok(diff_output_filename);

    // finish writing the cpio archive
    ofp->fputs("</revml>\n");
    ofp.reset();
    os_become_undo();
}
