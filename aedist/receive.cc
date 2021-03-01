//
//	aegis - project change supervisor
//	Copyright (C) 1999-2004 Peter Miller;
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
// MANIFEST: functions to receive change sets
//

#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/unistd.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <cattr.h>
#include <change.h>
#include <change/attributes.h>
#include <change/file.h>
#include <error.h>	// for assert
#include <fattr.h>
#include <help.h>
#include <input/cpio.h>
#include <move_list.h>
#include <open.h>
#include <os.h>
#include <output/bit_bucket.h>
#include <output/file.h>
#include <patch/list.h>
#include <project.h>
#include <project/file/trojan.h>
#include <project/file.h>
#include <project/history.h>
#include <receive.h>
#include <str.h>
#include <str_list.h>
#include <sub.h>
#include <undo.h>
#include <user.h>
#include <trace.h>

static long
to_long(string_ty *s)
{
    char            *end;
    long            result;

    result = strtol(s->str_text, &end, 10);
    if (*end)
	result = 0;;
    return result;
}

static void
move_xargs(string_ty *project_name, long change_number, string_list_ty *files,
           string_ty *dd)
{
    string_ty *fmt;

    if (0 != (files->nstrings % 2))
        this_is_a_bug();

    fmt =
        str_format
        (
            "aegis --move-file --project=%s --change=%ld --verbose",
            project_name->str_text,
            change_number
        );

    //
    // aemv needs an even number of file names, the os_xargs function
    // run the commands with, at most, 50 args.  Thus each aemv
    // invocation get an even number of file arguments.
    //
    os_xargs(fmt, files, dd);
}


static long
number_of_files(string_ty *project_name, long change_number)
{
    project_ty      *pp;
    change_ty       *cp;
    long            result;

    pp = project_alloc(project_name);
    project_bind_existing(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    result = change_file_count(cp);
    change_free(cp);
    project_free(pp);
    return result;
}


void
receive_main(void (*usage)(void))
{
    int		    use_patch;
    string_ty       *project_name;
    long            change_number;
    string_ty       *ifn;
    input_ty        *ifp;
    input_ty        *cpio_p;
    string_ty       *archive_name;
    string_ty       *s;
    project_ty      *pp;
    change_ty       *cp;
    string_ty       *dd;
    cstate_ty       *change_set;
    size_t          j;
    cattr_ty        *cattr_data;
    cattr_ty        *dflt;
    pconf_ty        *pconf_data;
    string_ty       *attribute_file_name;
    string_list_ty  files_source;
    string_list_ty  files_config;
    string_list_ty  files_build;
    string_list_ty  files_test_auto;
    string_list_ty  files_test_manual;
    move_list_ty    files_moved;
    int             could_have_a_trojan;
    int             config_seen;
    int             uncopy;
    int             trojan;
    string_ty       *dot;
    const char      *delta;
    string_ty       *devdir;
    int             exec_mode;
    int             non_exec_mode;
    string_list_ty  batch_moved;
    int             ignore_uuid;

    project_name = 0;
    change_number = 0;
    ifn = 0;
    trojan = -1;
    delta = 0;
    devdir = 0;
    use_patch = -1;
    ignore_uuid = -1;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    arglex_parse_change(&project_name, &change_number, usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, usage);
	    continue;

	case arglex_token_file:
	    if (ifn)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_file(arglex_token_file, usage);
	        // NOTREACHED

	    case arglex_token_string:
		ifn = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_stdio:
		ifn = str_from_c("");
		break;
	    }
	    break;

	case arglex_token_trojan:
	    if (trojan > 0)
		duplicate_option(usage);
	    if (trojan >= 0)
	    {
	        too_many_trojans:
		mutually_exclusive_options
		(
		    arglex_token_trojan,
		    arglex_token_trojan_not,
		    usage
		);
	    }
	    trojan = 1;
	    break;

	case arglex_token_trojan_not:
	    if (trojan == 0)
		duplicate_option(usage);
	    if (trojan >= 0)
		goto too_many_trojans;
	    trojan = 0;
	    break;

	case arglex_token_delta:
	    if (delta)
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_delta, usage);
		// NOTREACHED

	    case arglex_token_number:
	    case arglex_token_string:
		delta = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_directory:
	    if (devdir)
	    {
		duplicate_option(usage);
		// NOTREACHED
	    }
	    if (arglex() != arglex_token_string)
	    {
		option_needs_dir(arglex_token_directory, usage);
		// NOTREACHED
	    }
	    devdir = str_format(" --directory %s", arglex_value.alv_string);
	    break;

	case arglex_token_patch:
	    if (use_patch > 0)
		duplicate_option(usage);
	    if (use_patch >= 0)
	    {
	        too_many_patchs:
		mutually_exclusive_options
		(
		    arglex_token_patch,
		    arglex_token_patch_not,
		    usage
		);
	    }
	    use_patch = 1;
	    break;

	case arglex_token_patch_not:
	    if (use_patch == 0)
		duplicate_option(usage);
	    if (use_patch >= 0)
	        goto too_many_patchs;
	    use_patch = 0;
	    break;
        case arglex_token_ignore_uuid:
            if (ignore_uuid > 0)
                duplicate_option(usage);
            if (ignore_uuid >= 0)
            {
                too_many_ignore_uuid:
                mutually_exclusive_options
                (
                    arglex_token_ignore_uuid,
                    arglex_token_ignore_uuid_not,
                    usage
                );
            }
            ignore_uuid = 1;
            break;
        case arglex_token_ignore_uuid_not:
            if (ignore_uuid == 0)
                duplicate_option(usage);
            if (ignore_uuid >= 0)
                goto too_many_ignore_uuid;
            ignore_uuid = 0;
            break;
        }
        arglex();
    }

    //
    // Open the input file and verify the format.
    //
    cpio_p = aedist_open(ifn, (string_ty **) 0);
    assert(cpio_p);

    //
    // read the project name from the archive,
    // and use it to default the project if not given
    //
    os_become_orig();
    archive_name = 0;
    ifp = input_cpio_child(cpio_p, &archive_name);
    if (!ifp)
	input_fatal_error(cpio_p, "missing file");
    assert(archive_name);
    s = str_from_c("etc/project-name");
    if (!str_equal(archive_name, s))
	input_fatal_error(ifp, "wrong file");
    str_free(s);
    s = input_one_line(ifp);
    if (!s || !s->str_length)
	input_fatal_error(ifp, "short file");
    if (!project_name)
	project_name = s;
    else
	str_free(s);
    input_delete(ifp);
    os_become_undo();
    str_free(archive_name);

    //
    // locate project data
    //      (Even of we don't use it, this confirms it is a valid
    //      project name.)
    //
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    //
    // Read the change number form the archive, if it's there.  Use that
    // number (a) if the user didn't specify one on the command line,
    // and (b) that number is available.
    //
    os_become_orig();
    archive_name = 0;
    ifp = input_cpio_child(cpio_p, &archive_name);
    if (!ifp)
	input_fatal_error(cpio_p, "missing file");
    assert(archive_name);
    s = str_from_c("etc/change-number");
    if (str_equal(archive_name, s))
    {
	long            proposed_change_number;

	str_free(s);
	s = input_one_line(ifp);
	if (!s || !s->str_length)
	    input_fatal_error(ifp, "short file");
	proposed_change_number = to_long(s);
	str_free(s);
	input_delete(ifp);
	os_become_undo();
	str_free(archive_name);

	//
	// Make sure the change number is available.
	//
	if
	(
	    !change_number
	&&
	    proposed_change_number > 0
	&&
	    !project_change_number_in_use(pp, proposed_change_number)
	)
	    change_number = proposed_change_number;

	//
	// Start the next file, so we are in the same state as when
	// there is no change number included.
	//
	archive_name = 0;
	os_become_orig();
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (!ifp)
	    input_fatal_error(cpio_p, "missing file");
	assert(archive_name);
    }
    os_become_undo();

    //
    // default the change number
    //
    if (!change_number)
	change_number = project_next_change_number(pp, 1);

    //
    // get the change details from the input
    //
    os_become_orig();
    s = str_from_c("etc/change-set");
    if (!str_equal(s, archive_name))
	input_fatal_error(ifp, "wrong file");
    str_free(s);
    change_set = (cstate_ty *)parse_input(ifp, &cstate_type);
    ifp = 0;	// parse_input input_delete()ed it for us
    os_become_undo();
    str_free(archive_name);

    //
    // Make sure we like the change set at a macro level.
    //
    if
    (
	!change_set->brief_description
    ||
	!change_set->description
    ||
	!change_set->src
    ||
	!change_set->src->length
    )
	input_fatal_error(cpio_p, "bad change set");
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;

	src_data = change_set->src->list[j];
	if
	(
	    !src_data->file_name
	||
	    !src_data->file_name->str_length
	||
	    !(src_data->mask & cstate_src_action_mask)
	||
	    !(src_data->mask & cstate_src_usage_mask)
	)
	    input_fatal_error(cpio_p, "bad change info");
    }

    //
    // We refuse to receive a change we already have because that can
    // undo changes.  If the local repository is the source of the
    // change set, then the patch will fail to apply and the full
    // source will be used, thus successive changes will be removed.
    // The -ignore-uuid option is here to handle UUID clash, if any.
    //
    if (ignore_uuid <= 0 && change_set->uuid)
    {
        assert(universal_unique_identifier_valid(change_set->uuid));
        change_ty *c = project_uuid_find(pp, change_set->uuid);
        if (c)
        {
            assert(c->uuid);
            assert(universal_unique_identifier_valid(c->uuid));
            assert(str_equal(change_set->uuid, c->uuid));

	    //
	    // run away, run away!
	    //
	    error_intl(0, i18n("change already present"));
	    return;
        }
    }

    //
    // construct change attributes from the change_set
    //
    // Be careful when copying across the testing exemptions, to
    // make sure we don't ask for an exemption we can't have.
    //
    os_become_orig();
    attribute_file_name = os_edit_filename(0);
    undo_unlink_errok(attribute_file_name);
    cattr_data = (cattr_ty *)cattr_type.alloc();
    cattr_data->brief_description = str_copy(change_set->brief_description);
    cattr_data->description = str_copy(change_set->description);
    cattr_data->cause = change_set->cause;
    dflt = (cattr_ty *)cattr_type.alloc();
    dflt->cause = change_set->cause;
    os_become_undo();
    pconf_data = project_pconf_get(pp);
    change_attributes_default(dflt, pp, pconf_data);
    os_become_orig();
    cattr_data->test_exempt = (change_set->test_exempt && dflt->test_exempt);
    cattr_data->test_baseline_exempt =
	(change_set->test_baseline_exempt && dflt->test_baseline_exempt);
    cattr_data->regression_test_exempt =
	(change_set->regression_test_exempt && dflt->regression_test_exempt);
    if (change_set->attribute)
	cattr_data->attribute = attributes_list_copy(change_set->attribute);
    cattr_type.free(dflt);
    cattr_write_file(attribute_file_name, cattr_data, 0);
    cattr_type.free(cattr_data);
    project_free(pp);
    pp = 0;

    //
    // create the new change
    //
    dot = os_curdir();
    s =
	str_format
	(
	    "aegis --new-change %ld --project=%s --file=%s --verbose",
	    change_number,
	    project_name->str_text,
	    attribute_file_name->str_text
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_unlink_errok(attribute_file_name);

    //
    // Begin development of the new change.
    //
    s =
	str_format
	(
	    "aegis --develop-begin %ld --project %s --verbose%s",
	    change_number,
	    project_name->str_text,
	    (devdir ? devdir->str_text : "")
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_become_undo();

    //
    // Change to the development directory, so that we can use
    // relative filenames.  It makes things easier to read.
    //
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    dd = change_development_directory_get(cp, 0);
    dd = str_copy(dd);	// will vanish when change_free();
    change_free(cp);
    cp = 0;

    os_chdir(dd);

    //
    // Adjust the file actions to reflect the current state of
    // the project.
    //
    bool need_to_test = false;
    could_have_a_trojan = 0;
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;
	fstate_src_ty   *p_src_data;

	src_data = change_set->src->list[j];
	assert(src_data->file_name);
	trace(("%d %s %s\n", j, src_data->file_name->str_text,
	    file_action_ename(src_data->action)));
	p_src_data =
	    project_file_find(pp, src_data->file_name, view_path_extreme);
	switch (src_data->action)
	{
	case file_action_remove:
	    //
	    // Removing a removed file would be an
	    // error, so take it out of the list completely.
	    //
	    if (!p_src_data)
	    {
		size_t          k;

		//
                //  It's tempting to replace the removed element with
                //  the last and so shortening the list.  But in this
                //  way the order of the files in the change set
                //  differs from the order of the files in the cpio
                //  archive and this should not happen.
                //
		for (k = j + 1; k < change_set->src->length; ++k)
		    change_set->src->list[k - 1] = change_set->src->list[k];
		cstate_src_type.free(src_data);
		--j;
		change_set->src->length--;
		continue;
	    }
	    // the view_path_extreme was supposed to take care of this
	    assert(p_src_data->action != file_action_remove);
	    break;

	case file_action_transparent:
	    assert(0);

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	    if (!p_src_data)
	    {
		src_data->action = file_action_create;
	    }
	    else
	    {
		src_data->action = file_action_modify;
		switch (p_src_data->action)
		{
		case file_action_create:
		case file_action_modify:
		    break;

		case file_action_remove:
		    src_data->action = file_action_create;
		    break;

		case file_action_insulate:
		case file_action_transparent:
		    assert(0);
		    break;
		}
	    }
	    break;
	}
	if (project_file_trojan_suspect(pp, src_data->file_name))
	    could_have_a_trojan = 1;
    }

    //
    // add the removed files to the change
    //
    move_list_constructor(&files_moved);
    string_list_constructor(&files_source);
    string_list_constructor(&files_config);
    string_list_constructor(&files_build);
    string_list_constructor(&files_test_auto);
    string_list_constructor(&files_test_manual);

    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;
        fstate_src_ty   *p_src_data;

	//
	// For now, we are only removing files.
	//
	src_data = change_set->src->list[j];
	assert(src_data->file_name);
	switch (src_data->action)
	{
	case file_action_remove:
	    break;

	case file_action_modify:
            p_src_data =
		project_file_find(pp, src_data->file_name, view_path_extreme);
            assert (p_src_data);
            if (p_src_data->usage != src_data->usage)
	    {
		//
		// When files change type, it is necessary to remove
		// them *and* then create them in the same change.
		// Make sure the create loop also creates this file.
		//
                switch (p_src_data->usage)
                {
                case file_usage_config:
                    //
		    // We are receiving an archive that contains a file
		    // that our repository knows as file_usage_config,
		    // but deleting the last config file is forbidden
		    // and is wrong because we are receiving an old
		    // style archive.  Thus we skip the deletion.
                    //
                    continue;

                case file_usage_source:
                case file_usage_test:
                case file_usage_manual_test:
                case file_usage_build:
#ifndef DEBUG
		default:
#endif
                    src_data->action = file_action_create;
                    break;
                }
                break;
            }
            continue;

	case file_action_create:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;
	}

	//
	// add it to the list
	//
	if (src_data->move)
	{
	    move_list_append_remove
	    (
		&files_moved,
		src_data->file_name,
		src_data->move
	    );
	}
	else
	    string_list_append_unique(&files_source, src_data->file_name);
    }
    if (files_source.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --remove-file --project=%s --change=%ld --verbose",
		project_name->str_text,
		change_number
	    );
	os_xargs(s, &files_source, dd);
	str_free(s);
    }
    string_list_destructor(&files_source);

    //
    // add the modified files to the change
    //
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;

	//
	// For now, we are only copying files.
	//
	src_data = change_set->src->list[j];
	assert(src_data->file_name);
	switch (src_data->action)
	{
	case file_action_modify:
	    switch (src_data->usage)
	    {
	    case file_usage_build:
		break;

	    case file_usage_test:
	    case file_usage_manual_test:
		need_to_test = true;
		// fall through...

	    case file_usage_source:
	    case file_usage_config:
		string_list_append_unique(&files_source, src_data->file_name);
		break;
	    }
	    break;

	case file_action_create:
	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
    }
    uncopy = 0;
    if (files_source.nstrings)
    {
	string_ty       *delopt;

	delopt = 0;
	if (delta)
	{
	    delopt = str_from_c(delta);
	    s = str_quote_shell(delopt);
	    str_free(delopt);
	    delopt = str_format(" --delta=%s", s->str_text);
	    str_free(s);
	}
	uncopy = 1;
	s =
	    str_format
	    (
		"aegis --copy-file --project=%s --change=%ld --verbose%s",
		project_name->str_text,
		change_number,
		(delopt ? delopt->str_text : "")
	    );
	if (delopt)
	    str_free(delopt);
	os_xargs(s, &files_source, dd);
	str_free(s);
    }
    string_list_destructor(&files_source);


    //
    // add the new files to the change
    //
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;

	//
	// for now, we are only dealing with create
	//
	src_data = change_set->src->list[j];
	assert(src_data->file_name);
	switch (src_data->action)
	{
	case file_action_create:
	    break;

	case file_action_modify:
	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;
	}

	//
	// add it to the list
	//
	if (src_data->move)
	{
	    move_list_append_create
	    (
		&files_moved,
		src_data->move,
		src_data->file_name
	    );
	    continue;
	}
	switch (src_data->usage)
	{
	case file_usage_source:
	    string_list_append_unique(&files_source, src_data->file_name);
	    break;

	case file_usage_config:
	    string_list_append_unique(&files_config, src_data->file_name);
	    break;

	case file_usage_build:
	    string_list_append_unique(&files_build, src_data->file_name);
	    break;

	case file_usage_test:
	    string_list_append_unique(&files_test_auto, src_data->file_name);
	    need_to_test = true;
	    break;

	case file_usage_manual_test:
	    string_list_append_unique(&files_test_manual, src_data->file_name);
	    need_to_test = true;
	    break;
	}
    }

    if (files_build.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-file --build --project=%s --change=%ld "
		    "--verbose --no-template",
		project_name->str_text,
		change_number
	    );
	os_xargs(s, &files_build, dd);
	str_free(s);
    }
    if (files_test_auto.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-test --automatic --project=%s --change=%ld "
		    "--verbose --no-template",
		project_name->str_text,
		change_number
	    );
	os_xargs(s, &files_test_auto, dd);
	str_free(s);
    }
    if (files_test_manual.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-test --manual --project=%s --change=%ld "
		    "--verbose --no-template",
		project_name->str_text,
		change_number
	    );
	os_xargs(s, &files_test_manual, dd);
	str_free(s);
    }
    //
    // NOTE: do this one last, in case it includes the first instance
    // of the project config file.
    //
    if (files_source.nstrings)
    {
	s = str_from_c(THE_CONFIG_FILE_OLD);
	// FIXME: need to use the "is a config file" function
	if (string_list_member(&files_source, s))
	{
	    //
	    // The project config file must be created in the last set
	    // of files created, so move it to the end of the list.
	    //
	    string_list_remove(&files_source, s);
	    string_list_append(&files_source, s);
	}
	str_free(s);

	s =
	    str_format
	    (
		"aegis --new-file --project=%s --change=%ld --verbose "
		    "--no-template",
		project_name->str_text,
		change_number
	    );
	os_xargs(s, &files_source, dd);
	str_free(s);
    }

    //
    // NOTE: do this one last, in case it includes the first instance
    // of the project config file.
    //
    if (files_config.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-file --config --project=%s --change=%ld "
		    "--verbose --no-template",
		project_name->str_text,
		change_number
	    );
	os_xargs(s, &files_config, dd);
	str_free(s);
    }
    string_list_destructor(&files_source);
    string_list_destructor(&files_build);
    string_list_destructor(&files_test_auto);
    string_list_destructor(&files_test_manual);

    //
    // Now cope with files which moved.
    // They get a command each.
    //
    string_list_constructor(&batch_moved);
    for (j = 0; j < files_moved.length; ++j)
    {
	move_ty         *mp;

	mp = files_moved.item + j;
	if (mp->create)
	{
	    if (mp->remove)
	    {
                string_list_append(&batch_moved, mp->from);
                string_list_append(&batch_moved, mp->to);
	    }
	    else
	    {
		string_ty       *s1;
		string_ty       *s2;

                //
                // To maintain the order of the operation we run all
                // the move collected.
                //
                move_xargs(project_name, change_number, &batch_moved, dd);
                string_list_destructor(&batch_moved);
                string_list_constructor(&batch_moved);

		//
		// This can happen with older versions of Aegis if
		// the user does aemv, and then aermu of the source.
		// Recent versions of Aegis treat this as an error, so
		// we can't do what they did, we treat it as a simple
		// create instead.
		//
		s1 = str_quote_shell(mp->to);
		s2 =
		    str_format
		    (
			"aegis --new-file --project=%s --change=%ld "
			    "--no-template --verbose %s",
			project_name->str_text,
			change_number,
			s1->str_text
		    );
		str_free(s1);
                os_become_orig();
                os_execute(s2, OS_EXEC_FLAG_INPUT, dd);
                os_become_undo();
		str_free(s2);
	    }
	}
	else
	{
	    string_ty       *s1;
	    string_ty       *s2;

            //
            // To maintain the order of the operation we run all
            // the move collected.
            //
            move_xargs(project_name, change_number, &batch_moved, dd);
            string_list_destructor(&batch_moved);
            string_list_constructor(&batch_moved);

	    //
	    // This can happen with an older version of Aegis if the
	    // user does aemv, and then aenfu of the destination.
	    // Recent versions of Aegis treat this as an error, so
	    // we can't do what they did, we treat it as a simple
	    // remove instead.
	    //
	    assert(mp->create);
	    s1 = str_quote_shell(mp->from);
	    s2 =
		str_format
		(
		    "aegis --remove-file --project=%s --change=%ld "
			"--verbose %s",
		    project_name->str_text,
		    change_number,
		    s1->str_text
		);
	    str_free(s1);
            os_become_orig();
            os_execute(s2, OS_EXEC_FLAG_INPUT, dd);
            os_become_undo();
	    str_free(s2);
	}
    }

    //
    // We run any pending move.
    //
    move_xargs(project_name, change_number, &batch_moved, dd);
    string_list_destructor(&batch_moved);
    move_list_destructor(&files_moved);

    //
    // now extract each file from the input
    //
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    os_become_orig();
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;
	output_ty       *ofp;
	int             need_whole_source;

	// verbose progress message here?
	src_data = change_set->src->list[j];
	switch (src_data->action)
	{
	case file_action_insulate:
	case file_action_remove:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;

	case file_action_create:
	case file_action_modify:
	    break;
	}
	assert(src_data->file_name);
	switch (src_data->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    could_have_a_trojan = 1;
	    break;

	case file_usage_source:
	    break;
	}
	archive_name = 0;
	ifp = input_cpio_child(cpio_p, &archive_name);
	if (!ifp)
	    input_fatal_error(cpio_p, "missing file");
	assert(archive_name);
	need_whole_source = 1;
	s = str_format("patch/%s", src_data->file_name->str_text);
	if (str_equal(archive_name, s))
	{
	    patch_list_ty   *plp;

	    str_free(s);

	    //
	    // We have a patch file, but we also know that a
	    // complete source follows.  We can apply the patch
	    // or discard it.  If we fail to apply it cleanly,
	    // we can always use the complete source which follows.
	    //
	    plp = patch_read(ifp, 0);
	    input_delete(ifp);

	    switch (src_data->action)
	    {
	    case file_action_create:
                if (!use_patch)
                    break;

                //
                // We handle only the create half of the rename.
                //
                if (!src_data->move)
                    break;

                assert(plp);
                assert(plp->length >= 0);
                if (plp->length > 0)
                {
                    patch_ty    *p;
                    string_ty   *orig;
                    int         ok;

                    //
		    // Apply the patch.
		    //
		    // The input file (to which the patch is applied)
		    // is in the change.
		    //
		    p = plp->item[0];
		    os_become_undo();
		    assert(pp);
                    trace_string(src_data->file_name->str_text);
		    orig = project_file_path(pp, src_data->move);
		    os_become_orig();
		    ok = patch_apply(p, orig, src_data->file_name);
		    str_free(orig);
		    if (ok)
			need_whole_source = 0;
		    else
		    {
			sub_context_ty  *scp;

			scp = sub_context_new();
			sub_var_set_string
			(
			    scp,
			    "File_Name",
			    src_data->file_name
			);
			error_intl
			(
			    scp,
			    i18n("warning: $filename patch not used")
			);
			sub_context_delete(scp);
		    }
                }
                else
		{
                    //
                    // The patch does not contains info, so this is a
                    // "clean" rename without changes to the new
                    // file.  Thus the whole source is not needed.
                    //
                    need_whole_source = 0;
		}
                break;

            case file_action_modify:
		if (use_patch && plp->length == 1)
		{
		    patch_ty	*p;
		    string_ty       *orig;
		    int             ok;

		    //
		    // Apply the patch.
		    //
		    // The input file (to which the patch is applied) may
		    // be found in the baseline.
		    //
		    p = plp->item[0];
		    os_become_undo();
		    assert(pp);
		    orig = project_file_path(pp, src_data->file_name);
		    os_become_orig();
		    ok = patch_apply(p, orig, src_data->file_name);
		    str_free(orig);
		    if (ok)
			need_whole_source = 0;
		    else
		    {
			sub_context_ty  *scp;

			scp = sub_context_new();
			sub_var_set_string
			(
			    scp,
			    "File_Name",
			    src_data->file_name
			);
			error_intl
			(
			    scp,
			    i18n("warning: $filename patch not used")
			);
			sub_context_delete(scp);
		    }
		}
		break;

	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
		break;
	    }
	    patch_list_delete(plp);

	    //
	    // The src file should be next.
	    //
	    archive_name = 0;
	    ifp = input_cpio_child(cpio_p, &archive_name);
	    if (!ifp)
		input_fatal_error(cpio_p, "missing file");
	    assert(archive_name);
	}
	s = str_format("src/%s", src_data->file_name->str_text);
	if (!str_equal(archive_name, s))
	    input_fatal_error(ifp, "wrong file");
	str_free(s);
	if (need_whole_source)
	    ofp = output_file_binary_open(src_data->file_name);
	else
	    ofp = output_bit_bucket();
	input_to_output(ifp, ofp);
	output_delete(ofp);
	input_delete(ifp);
	str_free(archive_name);
    }
    os_become_undo();

    //
    // Now chmod the executable files,
    // and set the file attributes.
    //
    exec_mode = 0755 & ~change_umask(cp);
    non_exec_mode = exec_mode & ~0111;
    os_become_orig();
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;

	src_data = change_set->src->list[j];
	if (src_data->attribute)
	{
	    fattr_ty        *fattr_data;
	    string_ty       *s2;

	    fattr_data = (fattr_ty *)fattr_type.alloc();
	    fattr_data->attribute = attributes_list_copy(src_data->attribute);
	    fattr_write_file(attribute_file_name, fattr_data, 0);
	    fattr_type.free(fattr_data);
	    s2 = str_quote_shell(src_data->file_name);
	    s =
		str_format
		(
		    "aegis --file-attributes --change=%ld --project=%s "
			"--file=%s --verbose %s --base-rel",
		    change_number,
		    project_name->str_text,
		    attribute_file_name->str_text,
		    s2->str_text
		);
	    str_free(s2);
	    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	    str_free(s);
	    os_unlink_errok(attribute_file_name);
	}

	switch (src_data->action)
	{
	case file_action_create:
	case file_action_modify:
	    break;

	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;
	}

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
	assert(src_data->file_name);

	os_chmod
	(
	    src_data->file_name,
	    (src_data->executable ? exec_mode : non_exec_mode)
	);
    }
    os_become_undo();
    str_free(attribute_file_name);

    //
    // Now check to see if any of them were config files.  We couldn't do
    // it before now, in case we got an inconsistent config combination.
    //
    config_seen = 0;
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty     *      src_data;

	src_data = change_set->src->list[j];
	switch (src_data->action)
	{
	case file_action_create:
	case file_action_modify:
	    break;

	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;
	}
	switch (src_data->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	    assert(src_data->file_name);
	    if (change_file_is_config(cp, src_data->file_name))
	    {
		could_have_a_trojan = 1;
		config_seen = 1;
	    }
	    break;

	case file_usage_config:
	    could_have_a_trojan = 1;
	    config_seen = 1;
	    break;

	case file_usage_test:
	case file_usage_manual_test:
	    could_have_a_trojan = 1;
	    break;
	}
    }
    change_free(cp);
    cp = 0;

    //
    // should be at end of input
    //
    os_become_orig();
    archive_name = 0;
    ifp = input_cpio_child(cpio_p, &archive_name);
    if (ifp)
	input_fatal_error(cpio_p, "archive too long");
    input_delete(cpio_p);
    os_become_undo();

    //
    // Now that all the files have been unpacked,
    // set the change's UUID.
    //
    // It is vaguely possible you have already downloaded this change
    // before, so we don't complain (OS_EXEC_FLAG_ERROK) if the command
    // fails.
    //
    // We intentionally set the UUID before the aecpu -unch.  The aechu
    // -unch will clear the UUID if the chnage's file inventory changes,
    // because it is no longer the same change set.
    //
    if (change_set->uuid)
    {
	string_ty       *quoted_uuid;

	quoted_uuid = str_quote_shell(change_set->uuid);
	s =
	    str_format
	    (
		"aegis --change-attr --uuid %s -change=%ld --project=%s",
		quoted_uuid->str_text,
		change_number,
		project_name->str_text
	    );
	str_free(quoted_uuid);
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT | OS_EXEC_FLAG_ERROK, dd);
	os_become_undo();
	str_free(s);
    }

    //
    // Un-copy any files which did not change.
    //
    // The idea is, if there are no files left, there is nothing
    // for this change to do, so cancel it.
    //
    // We intentionally set the UUID before the aecpu -unch.  The aechu
    // -unch will clear the UUID if the chnage's file inventory changes,
    // because it is no longer the same change set.
    //
    if (uncopy)
    {
	s =
	    str_format
	    (
		"aegis --copy-file-undo --unchanged --change=%ld --project=%s "
		    "--verbose",
		change_number,
		project_name->str_text
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);

	//
	// If there are no files left, we already have this change.
	//
	if (number_of_files(project_name, change_number) == 0)
	{
	    //
	    // get out of there
	    //
	    os_chdir(dot);

	    //
	    // stop developing the change
	    //
	    s =
		str_format
		(
		    "aegis --develop-begin-undo --change=%ld --project=%s "
			"--verbose",
		    change_number,
		    project_name->str_text
		);
	    os_become_orig();
	    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	    str_free(s);

	    //
	    // cancel the change
	    //
	    s =
		str_format
		(
		    "aegis --new-change-undo --change=%ld --project=%s "
			"--verbose",
		    change_number,
		    project_name->str_text
		);
	    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	    os_become_undo();
	    str_free(s);

	    //
	    // run away, run away!
	    //
	    error_intl(0, i18n("change already present"));
	    return;
	}
    }

    //
    // If the change could have a trojan horse in it, stop here with
    // a warning.  The user needs to look at it and check.
    //
    if (trojan > 0)
	could_have_a_trojan = 1;
    else if (trojan == 0)
    {
	error_intl(0, i18n("warning: potential trojan, proceeding anyway"));
	could_have_a_trojan = 0;
	config_seen = 0;
    }

    //
    // If the change could have a trojan horse in the project config
    // file, stop here with a warning.  Don't even difference the
    // change, because the trojan could be embedded in the diff
    // command.  The user needs to look at it and check.
    //
    // FIX ME: what if the aecpu got rid of it?
    //
    if (config_seen)
    {
	error_intl
	(
	    0,
	 i18n("warning: potential trojan, review before completing development")
	);

	//
	// Make sure we are using an appropriate architecture.	This is
	// one of the commonest problems when seeding an empty repository.
	//
	s =
	    str_format
	    (
		"aegis --change-attr --fix-arch --change=%ld --project=%s",
		change_number,
		project_name->str_text
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);
	return;
    }

    //
    // now diff the change
    //
    s =
	str_format
	(
	    "aegis --diff --no-merge --change=%ld --project=%s --verbose",
	    change_number,
	    project_name->str_text
	);
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    //
    // If the change could have a trojan horse in it, stop here with
    // a warning.  The user needs to look at it and check.
    //
    if (could_have_a_trojan)
    {
	error_intl
	(
	    0,
	 i18n("warning: potential trojan, review before completing development")
	);
	return;
    }

    //
    // Sleep for a second to make sure the derived files will have
    // mod-times strictly later than the source files, and that the aeb
    // timestamp will also be strictly later then the mod times for the
    // source files.
    //
    sleep(1);

    //
    // now build the change
    //
    s =
	str_format
	(
	    "aegis --build --change=%ld --project=%s --verbose",
	    change_number,
	    project_name->str_text
	);
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    //
    // Sleep for a second to make sure the aet timestamps will be
    // strictly later then the aeb timestamp.
    //
    sleep(1);

    //
    // re-read the change state data.
    //
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    cstate_ty *cstate_data = change_cstate_get(cp);

    //
    // now test the change
    //
    if (need_to_test && !cstate_data->test_exempt)
    {
	s =
	    str_format
	    (
		"aegis --test --change=%ld --project=%s --verbose",
		change_number,
		project_name->str_text
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);
    }
    if (need_to_test && !cstate_data->test_baseline_exempt)
    {
	s =
	    str_format
	    (
		"aegis --test --baseline --change=%ld --project=%s --verbose",
		change_number,
		project_name->str_text
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);
    }

    // always to a regession test?
    if (!cstate_data->regression_test_exempt)
    {
	s =
	    str_format
	    (
		"aegis --test --regression --change=%ld --project=%s --verbose",
		change_number,
		project_name->str_text
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);
    }

    change_free(cp);
    cp = 0;
    project_free(pp);
    pp = 0;

    //
    // end development (if we got this far!)
    //
    s =
	str_format
	(
	    "aegis --develop-end --change=%ld --project=%s --verbose",
	    change_number,
	    project_name->str_text
	);
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    // verbose success message here?
}
