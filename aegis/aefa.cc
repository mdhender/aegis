//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <common/ac/libintl.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/language.h>
#include <common/nstring.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <common/uuidentifier.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/attribute.h>
#include <libaegis/attrlistveri.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/fattr.h>
#include <libaegis/help.h>
#include <libaegis/io.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <libaegis/undo.h>
#include <libaegis/user.h>

#include <aegis/aefa.h>


static void
file_attributes_usage(void)
{
    const char *progname = progname_get();
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
	"       %s -File_ATtributes -Edit [ <option>... ] <filename>\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -File_ATtributes name=value <filename>\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -File_ATtributes -List [ <option>... ] <filename>\n",
	progname
    );
    fprintf
    (
	stderr,
	"       %s -File_ATtributes -UUID <number> -File <filename>\n",
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
change_fatal_unknown_file(change::pointer cp, string_ty *filename)
{
    sub_context_ty sc(__FILE__, __LINE__);
    sc.var_set_string("File_Name", filename);
    fstate_src_ty *src = change_file_find_fuzzy(cp, filename);
    if (src)
    {
	sc.var_set_string("Guess", src->file_name);
	change_fatal(cp, &sc, i18n("no $filename, closest is $guess"));
	// NOTREACHED
    }
    else
    {
	change_fatal(cp, &sc, i18n("no $filename"));
	// NOTREACHED
    }
}


static void
fattr_assign(fattr_ty *fattr_data, const nstring &name, const nstring &value)
{
    attributes_list_ty *alp = fattr_data->attribute;
    if (!alp)
    {
	alp = (attributes_list_ty *)attributes_list_type.alloc();
	fattr_data->attribute = alp;
    }
    attributes_list_insert(alp, name.c_str(), value.c_str());
}


static bool
fattr_exists(fattr_ty *fattr_data, const nstring &name)
{
    return !!attributes_list_find(fattr_data->attribute, name.c_str());
}


static fattr_ty *
fattr_construct(fstate_src_ty *src)
{
    fattr_ty *fattr_data = (fattr_ty *)fattr_type.alloc();
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
    change::pointer cp;
    user_ty::pointer up;
    string_ty       *filename;
    fstate_src_ty   *src;

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
    	    user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
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
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Resolve the file name.
    //
    filename = change_file_resolve_name(cp, up, filename);

    //
    // build the fattr data
    //
    src = change_file_find(cp, filename, view_path_first);
    if (!src)
	change_fatal_unknown_file(cp, filename);
    fattr_data = fattr_construct(src);

    //
    // For changes which are still being developed, we also add the
    // "Content-Type" attribute.
    //
    if
    (
	cp->is_being_developed()
    &&
	!fattr_exists(fattr_data, "Content-Type")
    )
    {
	nstring path(change_file_path(cp, filename));
	os_become_orig();
	nstring content_type = os_magic_file(path);
	os_become_undo();
	assert(!content_type.empty());
	fattr_assign(fattr_data, "content-type", content_type);
    }

    //
    // print the fattr data
    //
    language_human();
    fattr_write_file((string_ty *)0, fattr_data, 0);
    language_C();
    fattr_type.free(fattr_data);
    project_free(pp);
    change_free(cp);
    trace(("}\n"));
}


static void
check_permissions(change::pointer cp, user_ty::pointer up)
{
    if
    (
	!cp->is_being_developed()
    ||
	nstring(change_developer_name(cp)) != up->name()
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
fattr_extract(fattr_ty *fattr_data, const nstring &name)
{
    return attributes_list_extract(fattr_data->attribute, name.c_str());
}


static void
file_attributes_main(void)
{
    sub_context_ty  *scp;
    string_ty	    *project_name;
    project_ty	    *pp;
    fattr_ty	    *fattr_data;
    long	    change_number;
    change::pointer cp;
    user_ty::pointer up;
    edit_ty	    edit;
    string_ty	    *inp;
    string_ty	    *filename;
    fstate_src_ty   *src;

    trace(("file_attributes_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    edit = edit_not_set;
    fattr_data = 0;
    inp = 0;
    filename = 0;
    nstring_list name_value_pairs;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(file_attributes_usage);
	    continue;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
    	    user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
	    break;

	case arglex_token_string:
	    if (strchr(arglex_value.alv_string, '='))
	    {
		name_value_pairs.push_back(arglex_value.alv_string);
		break;
	    }
            if (filename)
		fatal_too_many_files();
	    filename = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_file:
	    if (inp)
		duplicate_option(file_attributes_usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, file_attributes_usage);
		// NOTREACHED

	    case arglex_token_string:
		inp = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		inp = str_from_c("");
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
	    continue;

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
	    user_ty::lock_wait_argument(file_attributes_usage);
	    break;
	}
	arglex();
    }
    if (!filename)
    {
	error_intl(0, i18n("no file names"));
	file_attributes_usage();
    }
    if (inp)
    {
	os_become_orig();
	fattr_data = fattr_read_file(inp);
	os_become_undo();
	assert(fattr_data);
    }
    if (name_value_pairs.empty())
    {
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
    }
    else
    {
	if (fattr_data || edit != edit_not_set)
	    file_attributes_usage();
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
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // locate change data
    //
    if (!change_number)
	change_number = up->default_change(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);

    //
    // Resolve the file name.
    //
    filename = change_file_resolve_name(cp, up, filename);

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
	// For changes which are still being developed, we also add the
	// "content-type" attribute.
	//
	if
	(
	    cp->is_being_developed()
	&&
	    !fattr_exists(fattr_data, "content-type")
	)
	{
	    nstring path(change_file_path(cp, filename));
	    os_become_orig();
	    nstring content_type = os_magic_file(path);
	    os_become_undo();
	    fattr_assign(fattr_data, "content-type", content_type);
	}

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
    if (name_value_pairs.empty())
    {
	if (src->attribute)
	{
	    attributes_list_type.free(src->attribute);
	    src->attribute = 0;
	}
	if (fattr_data->attribute)
	{
	    //
	    // We need to extract the "usage" pseudo-attribute,
	    // and assign it to the file's usage if it is legal.
	    //
	    attributes_ty *ap = fattr_extract(fattr_data, "usage");
	    if (ap)
	    {
		if (ap->value)
		{
		    file_usage_type.enum_parse(ap->value, &src->usage);
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
    }
    else
    {
	if (!src->attribute)
	    src->attribute = (attributes_list_ty *)attributes_list_type.alloc();
	for (size_t j = 0; j < name_value_pairs.size(); ++j)
	{
	    nstring pair = name_value_pairs[j];
	    const char *eqp = strchr(pair.c_str(), '=');
	    assert(eqp);
	    if (eqp)
	    {
		nstring name(pair.c_str(), eqp - pair.c_str());
		nstring value(eqp + 1);

		//
                // Note that this will replace the first attribute with
                // that name.  If there is more than one of that name,
                // the second and subsequent attributes are unchanged.
                // If there is no attribute of that name, it will be
                // appended.
		//
		attributes_list_insert
		(
		    src->attribute,
		    name.c_str(),
		    value.c_str()
		);
	    }
	}
    }

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
    trace(("}\n"));
}


static void
file_attributes_uuid(void)
{
    trace(("file_attributes_uuid()\n{\n"));
    arglex();
    change_identifier cid;
    string_ty *uuid = 0;
    string_ty *filename = 0;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(file_attributes_usage);
	    continue;

	case arglex_token_string:
	    if (!uuid)
	    {
		uuid = str_from_c(arglex_value.alv_string);
		if (!universal_unique_identifier_valid(uuid))
		    option_needs_uuid(arglex_token_uuid, file_attributes_usage);
	    }
	    else if (!filename)
	    {
		filename = str_from_c(arglex_value.alv_string);
	    }
	    else
	    {
		duplicate_option_by_name
		(
		    arglex_token_uuid,
		    file_attributes_usage
		);
	    }
	    break;

	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_file, file_attributes_usage);
	    if (!filename)
	    {
		filename = str_from_c(arglex_value.alv_string);
	    }
	    else
	    {
		duplicate_option_by_name
		(
		    arglex_token_file,
		    file_attributes_usage
		);
	    }
	    break;

	case arglex_token_change:
	case arglex_token_number:
	case arglex_token_project:
	    cid.command_line_parse(file_attributes_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(file_attributes_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
    	    user_ty::relative_filename_preference_argument
            (
                file_attributes_usage
            );
	    break;
	}
	arglex();
    }
    if (!filename)
    {
	error_intl(0, i18n("no file names"));
	file_attributes_usage();
    }
    cid.command_line_check(file_attributes_usage);

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

    //
    // Resolve the file name.
    //
    filename = change_file_resolve_name(cid.get_cp(), cid.get_up(), filename);

    //
    // Unlike other change attributes, the UUID may *only* be edited by
    // the developer when the change is in the "being developed" state.
    // This is because it should only ever be done by aepatch or aedist,
    // immediately after the files have been unpacked.
    //
    if
    (
	!cid.get_cp()->is_being_developed()
    ||
	nstring(change_developer_name(cid.get_cp())) != cid.get_up()->name()
    )
    {
	change_fatal(cid.get_cp(), 0, i18n("bad fa, not auth"));
    }

    //
    // If the file already has a UUID, this command obviously isn't
    // being used by aepatch or aedist, so tell the human to take a hike.
    //
    fstate_src_ty *src =
	change_file_find(cid.get_cp(), filename, view_path_first);
    if (!src)
	change_fatal_unknown_file(cid.get_cp(), filename);
    if (src->uuid)
    {
	sub_context_ty *scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	change_fatal(cid.get_cp(), scp, i18n("$filename uuid already set"));
	// NOTREACHED
    }

    //
    // You can't set the UUID of a file that is not being created in
    // this change set.  Doing so would be too confusing for humans
    // looking at edit numbers, or looking at file histories, because it
    // would cause discontinuities in the lineages.
    //
    if (src->action != file_action_create)
    {
	change_fatal(cid.get_cp(), 0, i18n("bad fa, not auth"));
    }

    //
    // Make sure this file UUID has not been used before, anywhere in
    // the project.  For a genuine UUID this is unlikely, but humans
    // tend to do silly things at times, so this check is necessary.
    //
    fstate_src_ty *src2 =
	change_file_find(cid.get_cp(), uuid, view_path_simple);
    if (src2)
    {
	sub_context_ty *scp = sub_context_new();
	sub_var_set_string(scp, "Other", src2->file_name);
	sub_var_optional(scp, "Other");
	change_fatal(cid.get_cp(), scp, i18n("$filename uuid duplicate"));
	// NOTREACHED
    }

    //
    // Assign the UUID.  Can't do this before the has-it-been-used test,
    // or the UUID would show as a duplicate.
    //
    // Make sure it is in lower case.
    //
    src->uuid = str_downcase(uuid);
    str_free(uuid);
    uuid = 0;

    //
    // Write the cstate state back out.
    //
    change_cstate_write(cid.get_cp());
    commit();
    lock_release();
    sub_context_ty *scp = sub_context_new();
    sub_var_set_string(scp, "File_Name", filename);
    change_verbose(cid.get_cp(), scp, i18n("$filename attributes changed"));
    sub_context_delete(scp);
    trace(("}\n"));
}


void
file_attributes(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, file_attributes_help, 0 },
	{ arglex_token_list, file_attributes_list, 0 },
	{ arglex_token_uuid, file_attributes_uuid, 0 },
    };

    trace(("file_attributes()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), file_attributes_main);
    trace(("}\n"));
}
