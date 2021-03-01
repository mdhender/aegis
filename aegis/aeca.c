/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to list and modify change attributes
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/libintl.h>

#include <aeca.h>
#include <arglex2.h>
#include <cattr.h>
#include <change.h>
#include <change/attributes.h>
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
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <uname.h>
#include <undo.h>
#include <user.h>


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
    string_ty	    *project_name;
    project_ty	    *pp;
    cattr	    cattr_data;
    cstate	    cstate_data;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    int		    description_only;

    trace(("change_attributes_list()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    description_only = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(change_attributes_usage);
	    continue;

	case arglex_token_change:
	    if (arglex() != arglex_token_number)
	    {
		option_needs_number
		(
		    arglex_token_change,
		    change_attributes_usage
		);
	    }
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    change_attributes_usage
		);
	    }
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (project_name)
		duplicate_option(change_attributes_usage);
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
	    	    arglex_token_project,
		    change_attributes_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_description_only:
	    if (description_only)
		duplicate_option(change_attributes_usage);
	    description_only = 1;
	    break;
	}
	arglex();
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
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * build the cattr data
     */
    cstate_data = change_cstate_get(cp);
    cattr_data = (cattr)cattr_type.alloc();
    change_attributes_copy(cattr_data, cstate_data);

    /*
     * print the cattr data
     */
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
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


static void
check_permissions(change_ty *cp, user_ty *up)
{
    project_ty	    *pp;
    cstate	    cstate_data;

    pp = cp->pp;
    cstate_data = change_cstate_get(cp);

    if
    (
	!project_administrator_query(pp, user_name(up))
    &&
	(
	    cstate_data->state != cstate_state_being_developed
	||
	    !str_equal(change_developer_name(cp), user_name(up))
	)
    )
    {
	change_fatal(cp, 0, i18n("bad ca, not auth"));
    }
}


static cattr
cattr_fix_arch(change_ty *cp)
{
    cstate	    cstate_data;
    cattr	    cattr_data;
    string_ty       *un;
    size_t          j;
    pconf	    pconf_data;

    /*
     * Extract current change attributes.
     */
    cstate_data = change_cstate_get(cp);
    cattr_data = (cattr)cattr_type.alloc();
    change_attributes_copy(cattr_data, cstate_data);

    /*
     * Toss out the architecture information.
     */
    if (cattr_data->architecture)
	cattr_architecture_list_type.free(cattr_data->architecture);
    cattr_data->architecture = cattr_architecture_list_type.alloc();

    /*
     * For each of the project architectures, add all of the mandatory
     * ones, and any of the optional ones that match, to the architecture
     * list.
     */
    pconf_data = change_pconf_get(cp, 0);
    un = uname_variant_get();
    for (j = 0; j < pconf_data->architecture->length; ++j)
    {
	pconf_architecture pca;

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
		cattr_architecture_list_type.list_parse
		(
		    cattr_data->architecture,
		    &type_p
		);
	    assert(type_p == &string_type);
	    *sp = str_copy(pca->name);

	    /*
	     * Only the first match is used.  This is consistent with
	     * how the change_architecture_name function works.
	     */
	    un = 0;
	}
    }

    /*
     * If we didn't find any architectures, set the list to the
     * "unspecified" architecture, which is the Aegis default.
     */
    if (cattr_data->architecture->length == 0)
    {
	type_ty		*type_p;
	string_ty	**sp;

	sp =
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
change_attributes_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *project_name;
    project_ty	    *pp;
    cattr	    cattr_data;
    cstate	    cstate_data;
    pconf	    pconf_data;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    edit_ty	    edit;
    int		    description_only;
    string_ty	    *input;
    int		    fix_architecture;

    trace(("change_attributes_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
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
		/*NOTREACHED*/

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
	    if (arglex() != arglex_token_number)
	    {
		option_needs_number
		(
		    arglex_token_change,
		    change_attributes_usage
		);
	    }
	    /* fall through... */

	case arglex_token_number:
	    if (change_number)
	    {
		duplicate_option_by_name
		(
		    arglex_token_change,
		    change_attributes_usage
		);
	    }
	    change_number = arglex_value.alv_number;
	    if (change_number == 0)
		change_number = MAGIC_ZERO;
	    else if (change_number < 1)
	    {
		scp = sub_context_new();
		sub_var_set_long(scp, "Number", change_number);
		fatal_intl(scp, i18n("change $number out of range"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    break;

	case arglex_token_project:
	    if (project_name)
		duplicate_option(change_attributes_usage);
	    if (arglex() != arglex_token_string)
	    {
		option_needs_name
		(
		    arglex_token_project,
		    change_attributes_usage
		);
	    }
	    project_name = str_from_c(arglex_value.alv_string);
	    break;

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
	    cattr_data = (cattr)cattr_type.alloc();
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
	cattr_data = (cattr)cattr_type.alloc();

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * locate change data
     */
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    /*
     * edit the attributes
     */
    if (fix_architecture)
    {
	/* Do nothing, yet */
    }
    else if (edit != edit_not_set)
    {
	/*
	 * make sure they are allowed to,
	 * to avoid a wasted edit
	 */
	check_permissions(cp, up);

	/*
	 * fill in any other fields
	 */
	cstate_data = change_cstate_get(cp);
	change_attributes_copy(cattr_data, cstate_data);

	/*
	 * edit the attributes
	 */
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
	    sub_var_set_string(scp, "Name", project_name_get(pp));
	    sub_var_set_long(scp, "Number", magic_zero_decode(change_number));
	    io_comment_append(scp, i18n("Project $name, Change $number"));
	    sub_context_delete(scp);
	    change_attributes_edit(&cattr_data, edit);
	}
    }

    /*
     * lock the change
     */
    change_cstate_lock_prepare(cp);
    lock_take();
    cstate_data = change_cstate_get(cp);
    pconf_data = change_pconf_get(cp, 0);

    if (fix_architecture)
	cattr_data = cattr_fix_arch(cp);

    /*
     * make sure they are allowed to
     * (even if edited, could have changed during edit)
     */
    check_permissions(cp, up);

    /*
     * copy the attributes across
     */
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
    if (project_administrator_query(pp, user_name(up)))
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
	    change_fatal(cp, 0, i18n("bad ca, no test exempt"));
	}
	else
	{
	    /*
	     * developers may remove exemptions
	     */
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

    /*
     * copy the architecture across
     */
    if (cattr_data->architecture && cattr_data->architecture->length)
    {
	string_list_ty	caarch;
	string_list_ty	pcarch;
	long		j;

	/*
	 * make sure they did not name architectures
	 * we have never heard of
	 */
	string_list_constructor(&caarch);
	for (j = 0; j < cattr_data->architecture->length; ++j)
	    string_list_append(&caarch, cattr_data->architecture->list[j]);

	string_list_constructor(&pcarch);
	assert(pconf_data->architecture);
	assert(pconf_data->architecture->length);
	for (j = 0; j < pconf_data->architecture->length; ++j)
	{
	    string_list_append
	    (
		&pcarch,
		pconf_data->architecture->list[j]->name
	    );
	}

	if (!string_list_subset(&caarch, &pcarch))
	    fatal_intl(0, i18n("bad ca, unknown architecture"));
	string_list_destructor(&pcarch);

	/*
	 * developers may remove architecture exemptions
	 * but may not grant them
	 */
	if (!project_administrator_query(pp, user_name(up)))
	{
	    string_list_ty  csarch;

	    string_list_constructor(&csarch);
	    for (j = 0; j < cstate_data->architecture->length; ++j)
	    {
		string_list_append(&csarch, cstate_data->architecture->list[j]);
	    }

	    if (!string_list_subset(&csarch, &caarch))
		fatal_intl(0, i18n("bad ca, no arch exempt"));
	    string_list_destructor(&csarch);
	}
	string_list_destructor(&caarch);

	/*
	 * copy the architecture names across
	 */
	change_architecture_clear(cp);
	for (j = 0; j < cattr_data->architecture->length; ++j)
	{
	    change_architecture_add(cp, cattr_data->architecture->list[j]);
	}
    }

    /*
     * copy the copyright years list across
     */
    if (cattr_data->copyright_years && cattr_data->copyright_years->length)
    {
	long		j;

	if (cstate_data->copyright_years)
	    cstate_copyright_years_list_type.free(cstate_data->copyright_years);
	cstate_data->copyright_years = cstate_copyright_years_list_type.alloc();
	for (j = 0; j < cattr_data->copyright_years->length; ++j)
	{
	    type_ty	    *type_p;
	    long	    *int_p;

	    int_p =
		cstate_copyright_years_list_type.list_parse
		(
		    cstate_data->copyright_years,
		    &type_p
		);
	    assert(type_p==&integer_type);
	    *int_p = cattr_data->copyright_years->list[j];
	}
    }

    /*
     * copy the previous version across
     */
    if (cattr_data->version_previous)
    {
	if (cstate_data->version_previous)
	    str_free(cstate_data->version_previous);
	cstate_data->version_previous = str_copy(cattr_data->version_previous);
    }

    cattr_type.free(cattr_data);
    change_cstate_write(cp);
    commit();
    lock_release();
    change_verbose(cp, 0, i18n("attributes changed"));
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


void
change_attributes(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, change_attributes_help, },
	{arglex_token_list, change_attributes_list, },
    };

    trace(("change_attributes()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), change_attributes_main);
    trace(("}\n"));
}
