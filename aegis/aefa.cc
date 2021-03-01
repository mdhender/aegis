//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to list and modify file attributes
//

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/libintl.h>

#include <aefa.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <attrlistveri.h>
#include <change.h>
#include <change/file.h>
#include <commit.h>
#include <error.h>
#include <fattr.h>
#include <help.h>
#include <io.h>
#include <language.h>
#include <lock.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <sub.h>
#include <str_list.h>
#include <trace.h>
#include <undo.h>
#include <user.h>


static void
file_attributes_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -File_ATtributes -File <attr-file> [ <option>... ] "
	    "<filename>\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -File_ATtributes -Edit [ <option>... ] "
	    "<filename>\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -File_ATtributes -List [ <option>... ] "
	    "<filename>\n",
	progname
    );
    fprintf(stderr, "       %s -File_ATtributes -Help\n", progname);
    quit(1);
}


static void
file_attributes_help(void)
{
    help("aefa", file_attributes_usage);
}


static void
change_fatal_unknown_file(change_ty *cp, string_ty *filename)
{
    sub_context_ty  *scp;
    fstate_src_ty   *src;

    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    src = change_file_find_fuzzy(cp, filename);
    if (src)
    {
	sub_var_set_string(scp, "Guess", src->file_name);
	change_fatal(cp, scp, i18n("no $filename, closest is $guess"));
	// NOTREACHED
    }
    else
    {
	change_fatal(cp, scp, i18n("no $filename"));
	// NOTREACHED
    }
    sub_context_delete(scp);
}


static void
fattr_assign(fattr_ty *fattr_data, const char *name_c, const char *value_c)
{
    attributes_list_ty *alp;
    string_ty       *name;
    string_ty       *value;
    size_t          j;
    type_ty         *type_p;
    attributes_ty   *ap;
    attributes_ty   **app;

    alp = fattr_data->attribute;
    name = str_from_c(name_c);
    value = str_from_c(value_c);

    for (j = 0; j < alp->length; ++j)
    {
	ap = alp->list[j];
	if (ap->name && str_equal(ap->name, name))
	{
	    if (ap->value)
		str_free(ap->value);
	    ap->value = value;
	    str_free(name);
	    return;
	}
    }

    app = (attributes_ty **)attributes_list_type.list_parse(alp, &type_p);
    assert(type_p == &attributes_type);
    ap = (attributes_ty *)attributes_type.alloc();
    *app = ap;
    ap->name = name;
    ap->value = value;
}


static fattr_ty *
fattr_construct(fstate_src_ty *src)
{
    fattr_ty        *fattr_data;

    fattr_data = (fattr_ty *)fattr_type.alloc();
    if (src->attribute)
	fattr_data->attribute = attributes_list_copy(src->attribute);
    else
	fattr_data->attribute =
	    (attributes_list_ty *)attributes_list_type.alloc();

    fattr_assign(fattr_data, "usage", file_usage_ename(src->usage));
    return fattr_data;
}


static void
file_attributes_list(void)
{
    string_ty	    *project_name;
    project_ty	    *pp;
    fattr_ty	    *fattr_data;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    string_ty       *filename;
    string_ty       *s1;
    string_ty       *s2;
    fstate_src_ty   *src;
    int             based;
    string_ty       *base;
    size_t          k;
    string_list_ty  search_path;

    trace(("file_attributes_list()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    filename = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(file_attributes_usage);
	    continue;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
    	    user_relative_filename_preference_argument(file_attributes_usage);
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		file_attributes_usage
	    );
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, file_attributes_usage);
	    continue;

	case arglex_token_string:
	    filename = str_from_c(arglex_value.alv_string);
	    break;
	}
	arglex();
    }
    if (!filename)
    {
	error_intl(0, i18n("no file names"));
	file_attributes_usage();
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
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Resolve the file name.
    //
    change_search_path_get(cp, &search_path, 1);

    based =
        (
            search_path.nstrings >= 1
        &&
            (
                user_relative_filename_preference
                (
                    up,
                    uconf_relative_filename_preference_current
                )
            ==
                uconf_relative_filename_preference_base
            )
        );
    if (based)
        base = search_path.string[0];
    else
    {
        os_become_orig();
        base = os_curdir();
        os_become_undo();
    }

    s1 = filename;
    if (s1->str_text[0] == '/')
	s2 = str_copy(s1);
    else
	s2 = os_path_join(base, s1);
    user_become(up);
    s1 = os_pathname(s2, 1);
    user_become_undo();
    str_free(s2);
    s2 = 0;
    for (k = 0; k < search_path.nstrings; ++k)
    {
	s2 = os_below_dir(search_path.string[k], s1);
	if (s2)
	    break;
    }
    str_free(s1);
    if (!s2)
    {
	sub_context_ty  *scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	change_fatal(cp, scp, i18n("$filename unrelated"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    str_free(filename);
    filename = s2;

    //
    // build the fattr data
    //
    src = change_file_find(cp, filename, view_path_first);
    if (!src)
	change_fatal_unknown_file(cp, filename);
    fattr_data = fattr_construct(src);

    //
    // print the fattr data
    //
    language_human();
    fattr_write_file((string_ty *)0, fattr_data, 0);
    language_C();
    fattr_type.free(fattr_data);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


static void
check_permissions(change_ty *cp, user_ty *up)
{
    if
    (
	!change_is_being_developed(cp)
    ||
	!str_equal(change_developer_name(cp), user_name(up))
    )
    {
	change_fatal(cp, 0, i18n("bad fa, not auth"));
    }
}


static void
file_attributes_edit(fattr_ty **dp, edit_ty et)
{
    sub_context_ty  *scp;
    fattr_ty        *d;
    string_ty       *filename;
    string_ty       *msg;

    //
    // write attributes to temporary file
    //
    d = *dp;
    assert(d);
    filename = os_edit_filename(1);
    os_become_orig();
    language_human();
    fattr_write_file(filename, d, 0);
    language_C();
    fattr_type.free(d);

    //
    // an error message to issue if anything goes wrong
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    msg = subst_intl(scp, i18n("attributes in $filename"));
    sub_context_delete(scp);
    undo_message(msg);
    str_free(msg);
    os_become_undo();

    //
    // edit the file
    //
    os_edit(filename, et);

    //
    // read it in again
    //
    os_become_orig();
    d = fattr_read_file(filename);
    commit_unlink_errok(filename);
    str_free(filename);
    os_become_undo();
    attributes_list_verify(d->attribute);
    *dp = d;
}


static attributes_ty *
fattr_extract(fattr_ty *fattr_data, const char *name_c)
{
    attributes_list_ty *alp;
    string_ty       *name;
    size_t          j;

    alp = fattr_data->attribute;
    if (!alp)
	return 0;
    name = str_from_c("usage");
    for (j = 0; j < alp->length; ++j)
    {
	attributes_ty   *ap;

	ap = alp->list[j];
	if (!ap->name)
	    continue;
	if (str_equal(ap->name, name))
	{
	    alp->length--;
	    if (j < alp->length)
		alp->list[j] = alp->list[alp->length];
	    str_free(name);
	    return ap;
	}
    }
    str_free(name);
    return 0;
}


static void
file_attributes_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *project_name;
    project_ty	    *pp;
    fattr_ty	    *fattr_data;
    long	    change_number;
    change_ty	    *cp;
    user_ty	    *up;
    edit_ty	    edit;
    string_ty	    *input;
    string_ty	    *filename;
    string_ty	    *s1;
    string_ty	    *s2;
    int             based;
    string_ty       *base;
    size_t          k;
    fstate_src_ty   *src;
    string_list_ty  search_path;

    trace(("file_attributes_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    edit = edit_not_set;
    fattr_data = 0;
    input = 0;
    filename = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(file_attributes_usage);
	    continue;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
    	    user_relative_filename_preference_argument(file_attributes_usage);
	    break;

	case arglex_token_string:
	    if (filename)
		fatal_too_many_files();
	    filename = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_file:
	    if (input)
		duplicate_option(file_attributes_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, file_attributes_usage);
		// NOTREACHED

	    case arglex_token_string:
		input = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		input = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		file_attributes_usage
	    );
	    break;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, file_attributes_usage);
	    continue;

	case arglex_token_edit:
	    if (edit == edit_foreground)
		duplicate_option(file_attributes_usage);
	    if (edit != edit_not_set)
	    {
		too_many_edits:
		mutually_exclusive_options
		(
		    arglex_token_edit,
		    arglex_token_edit_bg,
		    file_attributes_usage
		);
	    }
	    edit = edit_foreground;
	    break;

	case arglex_token_edit_bg:
	    if (edit == edit_background)
		duplicate_option(file_attributes_usage);
	    if (edit != edit_not_set)
		goto too_many_edits;
	    edit = edit_background;
	    break;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(file_attributes_usage);
	    break;
	}
	arglex();
    }
    if (!filename)
    {
	error_intl(0, i18n("no file names"));
	file_attributes_usage();
    }
    if (input)
    {
	os_become_orig();
	fattr_data = fattr_read_file(input);
	os_become_undo();
	assert(fattr_data);
    }
    if (!fattr_data && edit == edit_not_set)
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
    if (edit != edit_not_set && !fattr_data)
    {
	fattr_data = (fattr_ty *)fattr_type.alloc();
	fattr_data->attribute =
	    (attributes_list_ty *)attributes_list_type.alloc();
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
    // locate user data
    //
    up = user_executing(pp);

    //
    // locate change data
    //
    if (!change_number)
	change_number = user_default_change(up);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Resolve the file name.
    //
    change_search_path_get(cp, &search_path, 1);

    based =
        (
            search_path.nstrings >= 1
        &&
            (
                user_relative_filename_preference
                (
                    up,
                    uconf_relative_filename_preference_current
                )
            ==
                uconf_relative_filename_preference_base
            )
        );
    if (based)
        base = search_path.string[0];
    else
    {
        os_become_orig();
        base = os_curdir();
        os_become_undo();
    }

    s1 = filename;
    if (s1->str_text[0] == '/')
	s2 = str_copy(s1);
    else
	s2 = os_path_join(base, s1);
    user_become(up);
    s1 = os_pathname(s2, 1);
    user_become_undo();
    str_free(s2);
    s2 = 0;
    for (k = 0; k < search_path.nstrings; ++k)
    {
	s2 = os_below_dir(search_path.string[k], s1);
	if (s2)
	    break;
    }
    str_free(s1);
    if (!s2)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	change_fatal(cp, scp, i18n("$filename unrelated"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    str_free(filename);
    filename = s2;

    //
    // edit the attributes
    //
    if (edit != edit_not_set)
    {
	//
	// make sure they are allowed to,
	// to avoid a wasted edit
	//
	check_permissions(cp, up);

	//
	// fill in any other fields
	//
	src = change_file_find(cp, filename, view_path_first);
	if (!src)
	    change_fatal_unknown_file(cp, filename);
	fattr_data = fattr_construct(src);

	//
	// edit the attributes
	//
	scp = sub_context_new();
	sub_var_set_string(scp, "Name", project_name_get(pp));
	sub_var_set_long(scp, "Number", magic_zero_decode(change_number));
	sub_var_set_string(scp, "File_Name", filename);
	io_comment_append
	(
	    scp,
	    i18n("Project $name, Change $number, File $filename")
	);
	sub_context_delete(scp);
	file_attributes_edit(&fattr_data, edit);
    }

    //
    // lock the change
    //
    change_cstate_lock_prepare(cp);
    lock_take();

    //
    // make sure they are allowed to
    // (even if edited, could have changed during edit)
    //
    check_permissions(cp, up);

    //
    // copy the attributes across
    //
    src = change_file_find(cp, filename, view_path_first);
    if (!src)
	change_fatal_unknown_file(cp, filename);
    if (src->attribute)
    {
	attributes_list_type.free(src->attribute);
	src->attribute = 0;
    }
    if (fattr_data->attribute)
    {
	attributes_ty   *ap;

	//
	// We need to extract the "usage" pseudo-attribute,
	// and assign it to the file's usage if it is legal.
	//
	ap = fattr_extract(fattr_data, "usage");
	if (ap)
	{
	    if (ap->value)
	    {
		file_usage_ty   tmp;

		tmp = (file_usage_ty)file_usage_type.enum_parse(ap->value);
		if (tmp != (file_usage_ty)(-1))
		    src->usage = tmp;
	    }
	    attributes_type.free(ap);
	}

	//
	// Copy the rest in the normal way.
	//
	if (fattr_data->attribute->length)
	    src->attribute = attributes_list_copy(fattr_data->attribute);
    }
    fattr_type.free(fattr_data);

    //
    // Write it all out again.
    //
    change_cstate_write(cp);
    commit();
    lock_release();
    scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    change_verbose(cp, scp, i18n("$filename attributes changed"));
    sub_context_delete(scp);
    project_free(pp);
    change_free(cp);
    user_free(up);
    trace(("}\n"));
}


void
file_attributes(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_help, file_attributes_help, },
	{arglex_token_list, file_attributes_list, },
    };

    trace(("file_attributes()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), file_attributes_main);
    trace(("}\n"));
}
