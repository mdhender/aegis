//
//	aegis - project change supervisor
//	Copyright (C) 1991-1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to list and modify change attributes
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/libintl.h>

#include <aeca.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <cattr.h>
#include <change.h>
#include <change/attributes.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/identifier.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <gmatch.h>
#include <help.h>
#include <io.h>
#include <language.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/history.h>
#include <quit.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <uname.h>
#include <undo.h>
#include <user.h>
#include <uuidentifier.h>


static void
change_attributes_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Change_Attributes -File <attr-file> [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Change_Attributes -Edit [ <option>... ]\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -Change_Attributes -List [ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Change_Attributes -Help\n", progname);
    quit(1);
}


static void
change_attributes_help(void)
{
    help("aeca", change_attributes_usage);
}


static void
change_attributes_list(void)
{
    cattr_ty	    *cattr_data;
    cstate_ty	    *cstate_data;
    int		    description_only;

    trace(("change_attributes_list()\n{\n"));
    arglex();
    description_only = 0;
    change_identifier cid;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_attributes_usage);
	    continue;

	case arglex_token_change:
	case arglex_token_number:
	case arglex_token_project:
	    cid.command_line_parse(change_attributes_usage);
	    continue;

	case arglex_token_description_only:
	    if (description_only)
		duplicate_option(change_attributes_usage);
	    description_only = 1;
	    break;
	}
	arglex();
    }
    cid.command_line_check(change_attributes_usage);

    //
    // build the cattr data
    //
    cstate_data = change_cstate_get(cid.get_cp());
    cattr_data = (cattr_ty *)cattr_type.alloc();
    change_attributes_copy(cattr_data, cstate_data);

    //
    // print the cattr data
    //
    language_human();
    if (description_only)
    {
	string_ty	*s;

	s = cattr_data->description;
	if (s && s->str_length)
	{
	    printf
	    (
		"%s%s",
		s->str_text,
		(s->str_text[s->str_length - 1] == '\n' ? "" : "\n")
	    );
	}
    }
    else
	cattr_write_file((string_ty *)0, cattr_data, 0);
    language_C();
    cattr_type.free(cattr_data);
    trace(("}\n"));
}


static void
check_permissions(change_identifier &cid)
{
    if
    (
	!project_administrator_query(cid.get_pp(), user_name(cid.get_up()))
    &&
	(
	    !change_is_being_developed(cid.get_cp())
	||
	    !str_equal
	    (
		change_developer_name(cid.get_cp()),
		user_name(cid.get_up())
	    )
	)
    )
    {
	change_fatal(cid.get_cp(), 0, i18n("bad ca, not auth"));
    }
}


static cattr_ty *
cattr_fix_arch(change_identifier &cid)
{
    cstate_ty	    *cstate_data;
    cattr_ty	    *cattr_data;
    string_ty       *un;
    size_t          j;
    pconf_ty        *pconf_data;

    //
    // Extract current change attributes.
    //
    cstate_data = change_cstate_get(cid.get_cp());
    cattr_data = (cattr_ty *)cattr_type.alloc();
    change_attributes_copy(cattr_data, cstate_data);

    //
    // Toss out the architecture information.
    //
    if (cattr_data->architecture)
	cattr_architecture_list_type.free(cattr_data->architecture);
    cattr_data->architecture =
        (cattr_architecture_list_ty *)cattr_architecture_list_type.alloc();

    //
    // For each of the project architectures, add all of the mandatory
    // ones, and any of the optional ones that match, to the architecture
    // list.
    //
    pconf_data = change_pconf_get(cid.get_cp(), 0);
    un = uname_variant_get();
    for (j = 0; j < pconf_data->architecture->length; ++j)
    {
	pconf_architecture_ty *pca;

	pca = pconf_data->architecture->list[j];
	if (!pca)
	    continue;
	if (!pca->name)
	    continue;
	if
	(
	    pca->mode == pconf_architecture_mode_required
	||
	    (
		pca->mode == pconf_architecture_mode_optional
	    &&
		un
	    &&
		gmatch(pca->pattern->str_text, un->str_text)
	    )
	)
	{
	    type_ty         *type_p;
	    string_ty       **sp;

	    sp =
		(string_ty **)
		cattr_architecture_list_type.list_parse
		(
		    cattr_data->architecture,
		    &type_p
		);
	    assert(type_p == &string_type);
	    *sp = str_copy(pca->name);

	    //
	    // Only the first match is used.  This is consistent with
	    // how the change_architecture_name function works.
	    //
	    un = 0;
	}
    }

    //
    // If we didn't find any architectures, set the list to the
    // "unspecified" architecture, which is the Aegis default.
    //
    if (cattr_data->architecture->length == 0)
    {
	type_ty		*type_p;
	string_ty	**sp;

	sp =
	    (string_ty **)
	    cattr_architecture_list_type.list_parse
	    (
	       	cattr_data->architecture,
	       	&type_p
	    );
	assert(type_p == &string_type);
	*sp = str_from_c("unspecified");
    }
    return cattr_data;
}


static void
change_attributes_uuid(void)
{
    cstate_ty	    *cstate_data;
    string_ty       *uuid;
    size_t          j;

    trace(("change_attributes_main()\n{\n"));
    arglex();
    change_identifier cid;
    uuid = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_attributes_usage);
	    continue;

	case arglex_token_string:
	    if (uuid)
	    {
		duplicate_option_by_name
		(
		    arglex_token_uuid,
		    change_attributes_usage
		);
	    }
	    uuid = str_from_c(arglex_value.alv_string);
	    if (!universal_unique_identifier_valid(uuid))
		option_needs_uuid(arglex_token_uuid, change_attributes_usage);
	    break;

	case arglex_token_change:
	case arglex_token_number:
	case arglex_token_project:
	    cid.command_line_parse(change_attributes_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(change_attributes_usage);
	    break;
	}
	arglex();
    }
    cid.command_line_check(change_attributes_usage);

    //
    // As a special case, if no UUID string was given,
    // make one up on the spot.
    //
    if (!uuid)
	uuid = universal_unique_identifier();

    //
    // lock the change
    //
    change_cstate_lock_prepare(cid.get_cp());
    lock_take();
    cstate_data = change_cstate_get(cid.get_cp());

    //
    // Unlike other change attributes, the UUID may *only* be edited by
    // the developer when the change is in the "being developed" state.
    // This is because it should only ever be done by aepatch or aedist,
    // immediately after the files have been unpacked.
    //
    if
    (
	!change_is_being_developed(cid.get_cp())
    ||
	!str_equal(change_developer_name(cid.get_cp()), user_name(cid.get_up()))
    )
    {
	change_fatal(cid.get_cp(), 0, i18n("bad ca, not auth"));
    }

    //
    // If the change already has a UUID, this command obviously isn't
    // being used by aepatch or aedist, so tell the human to take a hike.
    //
    if (cstate_data->uuid)
    {
	change_fatal(cid.get_cp(), 0, i18n("bad ca, uuid already set"));
    }

    //
    // Make sure the UUID has not been used before, anywhere in the
    // projects.  For a genuine UUID this is unlikely, but humans tend
    // to do silly things at times, so this expensive check will be
    // necessary.
    //
    change_ty *cp2 = project_uuid_find(cid.get_pp(), uuid);
    if (cp2)
    {
	sub_context_ty *scp = sub_context_new();
	sub_var_set_string(scp, "Other", change_version_get(cp2));
	sub_var_optional(scp, "Other");
	change_fatal(cid.get_cp(), scp, i18n("bad ca, uuid duplicate"));
	// NOTREACHED
    }

    //
    // If the file manifest of the change is altered (e.g. by aenf, aenfu,
    // aecp, aecpu, etc), or the contents of any file is changed, the
    // UUID is cleared.  This is because it is no longer the same change
    // as was received by aedist or aepatch, and the UUID is invalidated.
    //
    // At this point, we need to ensure that every change file has been
    // fingerprinted, to be able to implement the above condition.
    //
    os_throttle();
    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	src = change_file_nth(cid.get_cp(), j, view_path_first);
	if (!src)
	    break;
	change_file_fingerprint_check(cid.get_cp(), src);
    }

    //
    // Assign the UUID.  Can't do this before the has-it-been-used test,
    // or the UUID would show as a duplicate.  Can't do this before the
    // file fingerprint test, or it will be nuked.
    //
    // Make sure it is in lower case.
    //
    cstate_data->uuid = str_downcase(uuid);
    str_free(uuid);
    uuid = 0;

    //
    // Write the cstate state back out.
    //
    change_cstate_write(cid.get_cp());
    commit();
    lock_release();
    change_verbose(cid.get_cp(), 0, i18n("attributes changed"));
    trace(("}\n"));
}


static void
change_attributes_main(void)
{
    sub_context_ty  *scp;
    cattr_ty	    *cattr_data;
    cstate_ty	    *cstate_data;
    pconf_ty        *pconf_data;
    edit_ty	    edit;
    int		    description_only;
    string_ty	    *input;
    int		    fix_architecture;

    trace(("change_attributes_main()\n{\n"));
    arglex();
    change_identifier cid;
    edit = edit_not_set;
    description_only = 0;
    cattr_data = 0;
    input = 0;
    fix_architecture = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_attributes_usage);
	    continue;

	case arglex_token_string:
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_file)
	    );
	    error_intl(scp, i18n("warning: use $name option"));
	    sub_context_delete(scp);
	    if (input)
		fatal_too_many_files();
	    goto read_attr_file;

	case arglex_token_file:
	    if (input)
		duplicate_option(change_attributes_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, change_attributes_usage);
		// NOTREACHED

	    case arglex_token_string:
		read_attr_file:
		input = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		input = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_change:
	case arglex_token_number:
	case arglex_token_project:
	    cid.command_line_parse(change_attributes_usage);
	    continue;

	case arglex_token_edit:
	    if (edit == edit_foreground)
		duplicate_option(change_attributes_usage);
	    if (edit != edit_not_set)
	    {
		too_many_edits:
		mutually_exclusive_options
		(
		    arglex_token_edit,
		    arglex_token_edit_bg,
		    change_attributes_usage
		);
	    }
	    edit = edit_foreground;
	    break;

	case arglex_token_edit_bg:
	    if (edit == edit_background)
		duplicate_option(change_attributes_usage);
	    if (edit != edit_not_set)
		goto too_many_edits;
	    edit = edit_background;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(change_attributes_usage);
	    break;

	case arglex_token_description_only:
	    if (description_only)
		duplicate_option(change_attributes_usage);
	    description_only = 1;
	    break;

	case arglex_token_fix_architecture:
	    if (fix_architecture)
		duplicate_option(change_attributes_usage);
	    fix_architecture = 1;
	    break;
	}
	arglex();
    }
    cid.command_line_check(change_attributes_usage);
    if (fix_architecture)
    {
	if (edit == edit_foreground)
	{
	    mutually_exclusive_options
	    (
		arglex_token_edit,
		arglex_token_fix_architecture,
		change_attributes_usage
	    );
	}
	if (edit == edit_background)
	{
	    mutually_exclusive_options
	    (
		arglex_token_edit_bg,
		arglex_token_fix_architecture,
		change_attributes_usage
	    );
	}
	if (input)
	{
	    mutually_exclusive_options
	    (
		arglex_token_file,
		arglex_token_fix_architecture,
		change_attributes_usage
	    );
	}
    }
    if (input)
    {
	if (description_only)
	{
	    cattr_data = (cattr_ty *)cattr_type.alloc();
	    os_become_orig();
	    cattr_data->description = read_whole_file(input);
	    os_become_undo();
	}
	else
	{
	    os_become_orig();
	    cattr_data = cattr_read_file(input);
	    os_become_undo();
	}
	assert(cattr_data);
    }
    if (!cattr_data && edit == edit_not_set && !fix_architecture)
    {
	scp = sub_context_new();
	sub_var_set_charstar
	(
	    scp,
	    "Name1",
	    arglex_token_name(arglex_token_file)
	);
	sub_var_set_charstar
	(
	    scp,
	    "Name2",
	    arglex_token_name(arglex_token_edit)
	);
	error_intl(scp, i18n("warning: no $name1, assuming $name2"));
	sub_context_delete(scp);
	edit = edit_foreground;
    }
    if (edit != edit_not_set && !cattr_data)
	cattr_data = (cattr_ty *)cattr_type.alloc();

    //
    // edit the attributes
    //
    if (fix_architecture)
    {
	// Do nothing, yet
    }
    else if (edit != edit_not_set)
    {
	//
	// make sure they are allowed to,
	// to avoid a wasted edit
	//
	check_permissions(cid);

	//
	// fill in any other fields
	//
	cstate_data = change_cstate_get(cid.get_cp());
	change_attributes_copy(cattr_data, cstate_data);

	//
	// edit the attributes
	//
	if (description_only)
	{
	    string_ty	    *s;

	    s = os_edit_string(cattr_data->description, edit);
	    assert(s);
	    if (cattr_data->description)
		str_free(cattr_data->description);
	    cattr_data->description = s;
	}
	else
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", project_name_get(cid.get_pp()));
	    sub_var_set_long(scp, "Number", cid.get_change_number());
	    io_comment_append(scp, i18n("Project $name, Change $number"));
	    sub_context_delete(scp);
	    change_attributes_edit(&cattr_data, edit);
	}
    }

    //
    // lock the change
    //
    change_cstate_lock_prepare(cid.get_cp());
    lock_take();
    cstate_data = change_cstate_get(cid.get_cp());
    pconf_data = change_pconf_get(cid.get_cp(), 0);

    if (fix_architecture)
	cattr_data = cattr_fix_arch(cid);

    //
    // make sure they are allowed to
    // (even if edited, could have changed during edit)
    //
    check_permissions(cid);

    //
    // copy the attributes across
    //
    if (cattr_data->description)
    {
	if (cstate_data->description)
	    str_free(cstate_data->description);
	cstate_data->description = str_copy(cattr_data->description);
    }
    if (cattr_data->brief_description)
    {
	if (cstate_data->brief_description)
	    str_free(cstate_data->brief_description);
	cstate_data->brief_description =
	    str_copy(cattr_data->brief_description);
    }
    if (cattr_data->mask & cattr_cause_mask)
	cstate_data->cause = cattr_data->cause;
    if (project_administrator_query(cid.get_pp(), user_name(cid.get_up())))
    {
	if (cattr_data->mask & cattr_test_exempt_mask)
	{
	    cstate_data->test_exempt = cattr_data->test_exempt;
	    cstate_data->given_test_exemption = cattr_data->test_exempt;
	}
	if (cattr_data->mask & cattr_test_baseline_exempt_mask)
	{
	    cstate_data->test_baseline_exempt =
		cattr_data->test_baseline_exempt;
	}
	if (cattr_data->mask & cattr_regression_test_exempt_mask)
	{
	    cstate_data->regression_test_exempt =
		cattr_data->regression_test_exempt;
	    cstate_data->given_regression_test_exemption =
		cattr_data->regression_test_exempt;
	}
    }
    else
    {
	if
	(
	    (
		cattr_data->test_exempt
	    &&
		!cstate_data->test_exempt
	    &&
		!cstate_data->given_test_exemption
	    )
	||
	    (
		cattr_data->test_baseline_exempt
	    &&
		!cstate_data->test_baseline_exempt
	    )
	||
	    (
		cattr_data->regression_test_exempt
	    &&
		!cstate_data->regression_test_exempt
	    &&
		!cstate_data->given_regression_test_exemption
	    )
	)
	{
	    change_fatal(cid.get_cp(), 0, i18n("bad ca, no test exempt"));
	}
	else
	{
	    //
	    // developers may remove exemptions
	    //
	    if (cattr_data->mask & cattr_test_exempt_mask)
	    {
		cstate_data->test_exempt = cattr_data->test_exempt;
	    }
	    if (cattr_data->mask & cattr_test_baseline_exempt_mask)
	    {
		cstate_data->test_baseline_exempt =
		    cattr_data->test_baseline_exempt;
	    }
	    if (cattr_data->mask & cattr_regression_test_exempt_mask)
	    {
		cstate_data->regression_test_exempt =
		    cattr_data->regression_test_exempt;
	    }
	}
    }

    //
    // copy the architecture across
    //
    if (cattr_data->architecture && cattr_data->architecture->length)
    {
	//
	// make sure they did not name architectures
	// we have never heard of
	//
	string_list_ty caarch;
	for (size_t j = 0; j < cattr_data->architecture->length; ++j)
	    caarch.push_back(cattr_data->architecture->list[j]);

	string_list_ty pcarch;
	assert(pconf_data->architecture);
	assert(pconf_data->architecture->length);
	for (size_t k = 0; k < pconf_data->architecture->length; ++k)
	{
	    pcarch.push_back(pconf_data->architecture->list[k]->name);
	}

	if (!caarch.subset(pcarch))
	    fatal_intl(0, i18n("bad ca, unknown architecture"));

	//
	// developers may remove architecture exemptions
	// but may not grant them
	//
	if (!project_administrator_query(cid.get_pp(), user_name(cid.get_up())))
	{
	    string_list_ty csarch;
	    for (size_t m = 0; m < cstate_data->architecture->length; ++m)
	    {
		csarch.push_back(cstate_data->architecture->list[m]);
	    }

	    if (!csarch.subset(caarch))
		fatal_intl(0, i18n("bad ca, no arch exempt"));
	}

	//
	// copy the architecture names across
	//
	change_architecture_clear(cid.get_cp());
	for (size_t m = 0; m < cattr_data->architecture->length; ++m)
	{
	    change_architecture_add
	    (
		cid.get_cp(),
		cattr_data->architecture->list[m]
	    );
	}
    }

    //
    // copy the copyright years list across
    //
    if (cattr_data->copyright_years && cattr_data->copyright_years->length)
    {
	size_t		j;

	if (cstate_data->copyright_years)
	    cstate_copyright_years_list_type.free(cstate_data->copyright_years);
	cstate_data->copyright_years =
	    (cstate_copyright_years_list_ty *)
            cstate_copyright_years_list_type.alloc();
	for (j = 0; j < cattr_data->copyright_years->length; ++j)
	{
	    type_ty	    *type_p;
	    long	    *int_p;

	    int_p =
		(long int *)
		cstate_copyright_years_list_type.list_parse
		(
		    cstate_data->copyright_years,
		    &type_p
		);
	    assert(type_p==&integer_type);
	    *int_p = cattr_data->copyright_years->list[j];
	}
    }

    //
    // copy the previous version across
    //
    if (cattr_data->version_previous)
    {
	if (cstate_data->version_previous)
	    str_free(cstate_data->version_previous);
	cstate_data->version_previous = str_copy(cattr_data->version_previous);
    }

    //
    // Copy the user-defined attributes across.
    //
    if (cattr_data->attribute)
    {
	if (cstate_data->attribute)
	{
	    attributes_list_type.free(cstate_data->attribute);
	    cstate_data->attribute = 0;
	}
	if (cattr_data->attribute->length)
	{
	    cstate_data->attribute =
		attributes_list_copy(cattr_data->attribute);
	}
    }

    cattr_type.free(cattr_data);
    change_cstate_write(cid.get_cp());
    commit();
    lock_release();
    change_verbose(cid.get_cp(), 0, i18n("attributes changed"));
    trace(("}\n"));
}


void
change_attributes(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, change_attributes_help, },
	{arglex_token_list, change_attributes_list, },
	{arglex_token_uuid, change_attributes_uuid, },
    };

    trace(("change_attributes()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), change_attributes_main);
    trace(("}\n"));
}
