/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2002 Peter Miller;
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
 * MANIFEST: functions to manipulate gets
 */

#include <change.h>
#include <change/file.h>
#include <change/lock_sync.h>
#include <error.h>	/* for assert */
#include <fstate.h>
#include <input/catenate.h>
#include <input/crlf.h>
#include <input/file.h>
#include <mem.h>
#include <os.h>
#include <os/isa/path_prefix.h>
#include <project.h>
#include <str_list.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>


static void pconf_improve _((change_ty *, pconf, string_ty *));

static void
pconf_improve(cp, d, filename)
    change_ty       *cp;
    pconf           d;
    string_ty       *filename;
{
    sub_context_ty  *scp;
    size_t          j;

    if (!d->build_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "build_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!d->development_build_command)
	d->development_build_command = str_copy(d->build_command);
    if (!(d->mask & pconf_create_symlinks_before_integration_build_mask))
    {
	d->create_symlinks_before_integration_build =
	    d->create_symlinks_before_build;
	d->mask |= pconf_create_symlinks_before_integration_build_mask;
    }
    if (!(d->mask & pconf_remove_symlinks_after_integration_build_mask))
    {
	/*
	 * Integration builds always remove the symlinks
	 * again, even if they are kept around in the
	 * development directories.  This stops them
	 * becoming stale if there are deeper baseline
	 * integrations.
	 */
	d->remove_symlinks_after_integration_build = 1;
    }
    if (!d->history_create_command && d->history_put_command)
	d->history_create_command = str_copy(d->history_put_command);
    if (!d->history_create_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "history_create_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!d->history_get_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "history_get_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!d->history_put_command && d->history_create_command)
	d->history_put_command = str_copy(d->history_create_command);
    if (!d->history_put_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "history_put_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!d->history_query_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "history_query_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!d->diff_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "diff_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!d->diff3_command && !d->merge_command)
    {
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", filename);
	sub_var_set_charstar(scp, "FieLD_Name", "merge_command");
	change_fatal
	(
	    cp,
	    scp,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp);
    }
    if (!(d->mask & pconf_shell_safe_filenames_mask))
    {
	d->shell_safe_filenames = 1;
	d->mask |= pconf_shell_safe_filenames_mask;
    }
    if (d->file_template)
    {
	for (j = 0; j < d->file_template->length; ++j)
	{
	    pconf_file_template tp;

	    tp = d->file_template->list[j];
	    if (!tp->pattern || !tp->pattern->length)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		sub_var_set_charstar(scp, "FieLD_Name",
		    "file_template.pattern");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    if (!tp->body && !tp->body_command)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		sub_var_set_charstar(scp, "FieLD_Name", "file_template.body");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    if (tp->body && tp->body_command)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		sub_var_set_charstar(scp, "FieLD_Name", "file_template.body");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: corrupted \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	}
    }
    if (d->project_specific)
    {
	for (j = 0; j < d->project_specific->length; ++j)
	{
	    pconf_project_specific psp;

	    psp = d->project_specific->list[j];
	    if (!psp->name)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		sub_var_set_charstar(scp, "FieLD_Name",
		    "project_specific.name");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    if (!psp->value)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		sub_var_set_charstar(scp, "FieLD_Name",
		    "project_specific.value");
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	}
    }

    if (d->change_file_command)
    {
	if (!d->change_file_undo_command)
	    d->change_file_undo_command = str_copy(d->change_file_command);
	if (!d->new_file_command)
	    d->new_file_command = str_copy(d->change_file_command);
	if (!d->new_test_command)
	    d->new_test_command = str_copy(d->change_file_command);
	if (!d->copy_file_command)
	    d->copy_file_command = str_copy(d->change_file_command);
	if (!d->remove_file_command)
	    d->remove_file_command = str_copy(d->change_file_command);
    }
    if (d->change_file_undo_command)
    {
	if (!d->new_file_undo_command)
	    d->new_file_undo_command = str_copy(d->change_file_undo_command);
	if (!d->new_test_undo_command)
	    d->new_test_undo_command = str_copy(d->change_file_undo_command);
	if (!d->copy_file_undo_command)
	    d->copy_file_undo_command = str_copy(d->change_file_undo_command);
	if (!d->remove_file_undo_command)
	    d->remove_file_undo_command = str_copy(d->change_file_undo_command);
    }
}


static void set_pconf_symlink_exceptions_defaults _((pconf));

static void
set_pconf_symlink_exceptions_defaults(pconf_data)
    pconf           pconf_data;
{
    type_ty         *type_p;
    string_ty       **str_p;

    /*
     * make sure the list is there
     */
    if (!pconf_data->symlink_exceptions)
	pconf_data->symlink_exceptions =
	    pconf_symlink_exceptions_list_type.alloc();

    /*
     * append the logfile to the list
     */
    str_p =
	pconf_symlink_exceptions_list_type.list_parse
	(
	    pconf_data->symlink_exceptions,
	    &type_p
	);
    assert(type_p == &string_type);
    *str_p = str_copy(change_logfile_basename());
}


static input_ty *input_catenate_tricky _((string_list_ty *));

static input_ty *
input_catenate_tricky(filename)
    string_list_ty  *filename;
{
    input_ty        **fpl;
    size_t          j;
    input_ty        *fp;

    assert(filename->nstrings >= 1);
    fpl = mem_alloc(filename->nstrings * sizeof(fpl[0]));
    for (j = 0; j < filename->nstrings; ++j)
    {
	fpl[j] = input_crlf(input_file_open(filename->string[j]), 1);
    }
    fp = input_catenate(fpl, filename->nstrings, 1);
    mem_free(fpl);
    return fp;
}


static pconf read_the_pconf_list _((change_ty *, string_ty *));

static pconf
read_the_pconf_list(cp, dirname)
    change_ty       *cp;
    string_ty       *dirname;
{
    symtab_ty       *stp;
    string_ty       *s;
    project_ty      *pp;
    input_ty        *ifp;
    string_list_ty  filename;
    size_t          j;
    fstate_src      src;
    pconf           result;

    /*
     * If there is no config directory,
     * return a NULL pointer.
     */
    if (!dirname || !dirname->str_length)
	return 0;

    /*
     * Build the list of files.
     *
     * We use a symbol table so that we get O(n) behaviour.
     * (The string_list_append_list_unique is O(n**2), oops.)
     */
    stp = symtab_alloc(100);
    string_list_constructor(&filename);
    string_list_append(&filename, change_pconf_path_get(cp));
    for (j = 0;; ++j)
    {
	src = change_file_nth(cp, j);
	if (!src)
	    break;
	if (src->about_to_be_created_by)
	    continue;
	if (src->deleted_by)
	    continue;
	if (src->about_to_be_copied_by)
	    continue;
	if (!os_isa_path_prefix(dirname, src->file_name))
	    continue;
	s = change_file_path(cp, src->file_name);
	assert(s);
	symtab_assign(stp, src->file_name, s);
	string_list_append(&filename, s);
    }
    for (pp = cp->pp; pp; pp = pp->parent)
    {
	change_ty      *cp2;

	cp2 = project_change_get(pp);
	for (j = 0;; ++j)
	{
	    src = change_file_nth(cp2, j);
	    if (!src)
		break;
	    if (src->about_to_be_created_by)
		continue;
	    if (src->deleted_by)
		continue;
	    if (src->about_to_be_copied_by)
		continue;
	    if (!os_isa_path_prefix(dirname, src->file_name))
		continue;
	    if (symtab_query(stp, src->file_name))
		continue;
	    s = change_file_path(cp2, src->file_name);
	    assert(s);
	    symtab_assign(stp, src->file_name, s);
	    string_list_append(&filename, s);
	}
    }
    symtab_free(stp);

    /*
     * If there are no candidate files,
     * return a NULL pointer.
     */
    if (filename.nstrings <= 1)
    {
	string_list_destructor(&filename);
	return 0;
    }

    change_become(cp);
    ifp = input_catenate_tricky(&filename);
    result = parse_input(ifp, &pconf_type);
    /* as a side-effect, parse_input will delete fp */
    change_become_undo();

    return result;
}


pconf
change_pconf_get(cp, required)
    change_ty       *cp;
    int             required;
{
    static string_ty *star_comma_d;
    size_t          j;

    trace(("change_pconf_get(cp = %8.8lX, required = %d)\n{\n",
	(long)cp, required));
    assert(cp->reference_count >= 1);
    change_lock_sync(cp);
    if (!cp->pconf_data)
    {
	string_ty       *filename;
	int		ok;

	filename = change_pconf_path_get(cp);
	assert(filename);
	change_become(cp);
	ok = os_exists(filename);
	change_become_undo();
	if (!ok)
	{
	    if (required)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_charstar(scp, "File_Name", THE_CONFIG_FILE);
		change_fatal
		(
		    cp,
		    scp,
		    i18n("you must create a \"$filename\" file")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    cp->pconf_data = pconf_type.alloc();
	    cp->pconf_data->shell_safe_filenames = 1;
	}
	else
	{
	    change_become(cp);
	    cp->pconf_data = pconf_read_file(filename);
	    change_become_undo();
	    if (cp->pconf_data->configuration_directory)
	    {
		pconf           temp;

		temp =
		    read_the_pconf_list
		    (
			cp,
			cp->pconf_data->configuration_directory
		    );
		if (temp)
		{
		    pconf_type.free(cp->pconf_data);
		    cp->pconf_data = temp;
		}
	    }
	    pconf_improve(cp, cp->pconf_data, filename);
	}

	/*
	 * set the architecture default
	 */
	if (!cp->pconf_data->architecture)
	    cp->pconf_data->architecture = pconf_architecture_list_type.alloc();
	if (!cp->pconf_data->architecture->length)
	{
	    type_ty         *type_p;
	    pconf_architecture *app;
	    pconf_architecture ap;

	    app =
		pconf_architecture_list_type.list_parse
		(
		    cp->pconf_data->architecture,
		    &type_p
		);
	    assert(type_p == &pconf_architecture_type);
	    ap = pconf_architecture_type.alloc();
	    *app = ap;
	    ap->name = str_from_c("unspecified");
	    ap->pattern = str_from_c("*");
	}
	for (j = 0; j < cp->pconf_data->architecture->length; ++j)
	{
	    pconf_architecture ap;
	    ap = cp->pconf_data->architecture->list[j];
	    if (!ap->name && !ap->pattern)
	    {
		sub_context_ty *scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		change_fatal
		(
		    cp,
		    scp,
		    i18n("$filename: arch needs name and pattern")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }

	    /*
	     * Make sure the report generator can see it.
	     */
	    ap->mask |= pconf_architecture_mode_mask;
	}

	/*
	 * set the maximum_filename_length default
	 */
	if (cp->pconf_data->mask & pconf_maximum_filename_length_mask)
	{
	    if (cp->pconf_data->maximum_filename_length < 9)
		cp->pconf_data->maximum_filename_length = 9;
	    if (cp->pconf_data->maximum_filename_length > 255)
		cp->pconf_data->maximum_filename_length = 255;
	}
	else
	    cp->pconf_data->maximum_filename_length = 255;

	/*
	 * set the filename_pattern_accept default
	 */
	if (!cp->pconf_data->filename_pattern_accept)
	    cp->pconf_data->filename_pattern_accept =
		pconf_filename_pattern_accept_list_type.alloc();
	if (!cp->pconf_data->filename_pattern_accept->length)
	{
	    type_ty         *type_p;
	    string_ty       **addr_p;

	    addr_p =
		pconf_filename_pattern_accept_list_type.list_parse
		(
		    cp->pconf_data->filename_pattern_accept,
		    &type_p
		);
	    assert(type_p == &string_type);
	    *addr_p = str_from_c("*");
	}
	if (!star_comma_d)
	    star_comma_d = str_from_c("*,D");
	if (!cp->pconf_data->filename_pattern_reject)
	    cp->pconf_data->filename_pattern_reject =
		pconf_filename_pattern_reject_list_type.alloc();
	for (j = 0; j < cp->pconf_data->filename_pattern_reject->length; ++j)
	{
	    if
	    (
		str_equal
		(
		    star_comma_d,
		    cp->pconf_data->filename_pattern_reject->list[j]
		)
	    )
		break;
	}
	if (j >= cp->pconf_data->filename_pattern_reject->length)
	{
	    type_ty         *type_p;
	    string_ty       **addr_p;

	    addr_p =
		pconf_filename_pattern_reject_list_type.list_parse
		(
		    cp->pconf_data->filename_pattern_reject,
		    &type_p
		);
	    assert(type_p == &string_type);
	    *addr_p = str_copy(star_comma_d);
	}

	/*
	 * make sure symlink_exceptions is there, even if empty
	 */
	set_pconf_symlink_exceptions_defaults(cp->pconf_data);

	/*
	 * set the test_command default
	 */
	if (!cp->pconf_data->test_command)
	    cp->pconf_data->test_command = str_from_c("$shell $file_name");
	if (!cp->pconf_data->development_test_command)
	    cp->pconf_data->development_test_command =
		str_copy(cp->pconf_data->test_command);

	/*
	 * set the development directory template default
	 */
	if (!cp->pconf_data->development_directory_template)
	{
	    cp->pconf_data->development_directory_template =
		str_from_c
		(
"$ddd/${left $project ${expr ${namemax $ddd} - ${length .$magic${zpad $c 3}}}}\
.$magic${zpad $c 3}"
		);
	}

	/*
	 * Set the test filename template
	 */
	if (!cp->pconf_data->new_test_filename)
	{
	    cp->pconf_data->new_test_filename =
		str_from_c
		(
		  "test/${zpad $hundred 2}/t${zpad $number 4}${left $type 1}.sh"
		);
	}

	/*
	 * Make sure the report generator can see the enums.
	 */
	cp->pconf_data->mask |=
	    pconf_history_put_trashes_file_mask |
	    pconf_history_content_limitation_mask
	    ;
    }
    trace(("return %08lX;\n", (long)cp->pconf_data));
    trace(("}\n"));
    return cp->pconf_data;
}
