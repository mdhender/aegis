//
//	aegis - project change supervisor
//	Copyright (C) 1999-2005 Peter Miller;
//	Copyright (C) 2005 Walter Franzini;
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
#include <attribute.h>
#include <cattr.h>
#include <change.h>
#include <change/attributes.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/functor/invent_build.h>
#include <error.h>	// for assert
#include <fattr.h>
#include <help.h>
#include <input/cpio.h>
#include <project/invento_walk.h>
#include <move_list.h>
#include <nstring.h>
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
#include <symtab/template.h>
#include <undo.h>
#include <user.h>
#include <usage.h>
#include <uuidentifier.h>
#include <version_stmp.h>
#include <trace.h>


static void
move_xargs(string_ty *project_name, long change_number, string_list_ty *files,
    string_ty *dd, const nstring &trace_options)
{
    string_ty *fmt;

    if (0 != (files->nstrings % 2))
        this_is_a_bug();

    fmt =
        str_format
        (
            "aegis --move-file --project=%s --change=%ld%s --verbose",
            project_name->str_text,
            change_number,
            trace_options.c_str()
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


static fstate_src_ty *
project_file_find_by_meta2(project_ty *pp, cstate_src_ty *c_src_data,
    view_path_ty vp)
{
    trace(("project_file_find_by_meta2(pp = %08lX, c_src = %08lX, vp = %s)\n"
	"{\n", (long)pp, (long)c_src_data, view_path_ename(vp)));
    trace
    ((
	"change: %s %s \"%s\" %s\n", file_usage_ename(c_src_data->usage),
	file_action_ename(c_src_data->action),
	c_src_data->file_name->str_text,
	(c_src_data->edit_number ?
	    c_src_data->edit_number->str_text : "")
    ));
#ifdef DEBUG
    if (c_src_data->move)
	trace(("  move \"%s\"\n", c_src_data->move->str_text));
#endif

    //
    // Try to find the file by its UUID.
    //
    // For backwards compatibility reasons (UUIDs were introduced in
    // 4.17) there could be a rename chain (pointing at UUID-less
    // entries) to be taken into account.
    //
    if (c_src_data->uuid)
    {
	trace(("uuid = %s\n", c_src_data->uuid->str_text));
	fstate_src_ty *p_src_data =
	    project_file_find_by_uuid(pp, c_src_data->uuid, vp);
	while (p_src_data)
	{
	    if (p_src_data->action != file_action_remove || !p_src_data->move)
	    {
		trace
		((
		    "project: %s %s \"%s\" %s %s\n",
		    file_usage_ename(p_src_data->usage),
		    file_action_ename(p_src_data->action),
		    p_src_data->file_name->str_text,
		    (p_src_data->edit_origin ?
			p_src_data->edit_origin->revision->str_text : ""),
		    (p_src_data->edit ?
			p_src_data->edit->revision->str_text : "")
		));
		trace(("return %08lX;\n", (long)p_src_data));
		trace(("}\n"));
		return p_src_data;
	    }
	    p_src_data = project_file_find(pp, p_src_data->move, vp);
	}
    }

    //
    // Look for the file by name, tracking renames as they happen.
    //
    string_ty *name = c_src_data->file_name;
    if (c_src_data->action == file_action_create && c_src_data->move)
	name = c_src_data->move;
    for (;;)
    {
	fstate_src_ty *p_src_data = project_file_find(pp, name, vp);
	if (!p_src_data)
	{
	    trace(("return NULL;\n"));
	    trace(("}\n"));
	    return 0;
	}
	if (p_src_data->action != file_action_remove || !p_src_data->move)
	{
	    trace
	    ((
		"project: %s %s \"%s\" %s %s\n",
		file_usage_ename(p_src_data->usage),
		file_action_ename(p_src_data->action),
		p_src_data->file_name->str_text,
		(p_src_data->edit_origin ?
		    p_src_data->edit_origin->revision->str_text : ""),
		(p_src_data->edit ?
		    p_src_data->edit->revision->str_text : "")
	    ));
	    trace(("return %08lX;\n", (long)p_src_data));
	    trace(("}\n"));
	    return p_src_data;
	}
	name = p_src_data->move;
    }
}


static cstate_src_ty *
find_src(cstate_ty *cs, string_ty *filename)
{
    assert(cs);
    assert(cs->src);
    for (size_t j = 0; j < cs->src->length; ++j)
    {
	cstate_src_ty *src = cs->src->list[j];
	if (str_equal(src->file_name, filename))
	    return src;
    }
    return 0;
}


static void
wrong_file(input_ty *ifp, const nstring &expected)
{
    nstring message =
	nstring::format
	(
    	    "wrong file, expected \"%s\", "
    	    "maybe the sender needs to use the --compatibility=%s option",
    	    expected.c_str(),
    	    version_stamp()
	);
    ifp->fatal_error(message.c_str());
}


static void
missing_file(input_ty *ifp, const nstring &expected)
{
    nstring message =
	nstring::format
	(
    	    "missing file, expected \"%s\", "
    	    "maybe the sender needs to use the --compatibility=%s option",
    	    expected.c_str(),
    	    version_stamp()
	);
    ifp->fatal_error(message.c_str());
}


void
receive_main(void)
{
    int		    use_patch;
    string_ty       *project_name;
    long            change_number;
    string_ty       *ifn;
    input_ty        *ifp;
    project_ty      *pp;
    change_ty       *cp;
    string_ty       *dd;
    cstate_ty       *change_set;
    size_t          j;
    cattr_ty        *cattr_data;
    cattr_ty        *dflt;
    pconf_ty        *pconf_data;
    string_ty       *attribute_file_name;
    move_list_ty    files_moved;
    int             could_have_a_trojan;
    int             config_seen;
    int             uncopy;
    int             trojan;
    string_ty       *dot;
    string_ty       *devdir;
    int             exec_mode;
    int             non_exec_mode;
    int             ignore_uuid;

    project_name = 0;
    change_number = 0;
    ifn = 0;
    trojan = -1;
    nstring delta;
    devdir = 0;
    use_patch = -1;
    ignore_uuid = -1;
    arglex();
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
	    if (!delta.empty())
		duplicate_option(usage);
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_delta, usage);
		// NOTREACHED

	    case arglex_token_number:
		delta =
		    nstring::format(" --delta=%ld", arglex_value.alv_number);
		break;

	    case arglex_token_string:
		{
		    if (arglex_value.alv_string[0] == 0)
			option_needs_number(arglex_token_delta, usage);
		    nstring arg = arglex_value.alv_string;
		    arg = arg.quote_shell();
		    delta = nstring::format(" --delta=%s", arg.c_str());
		}
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
    input_cpio *cpio_p = aedist_open(ifn, (string_ty **) 0);
    assert(cpio_p);

    //
    // Calculate the --reason option to use with the
    // "aegis --new-change" command, if any.
    //
    nstring reason;
    if (cpio_p->is_remote())
    {
	reason =
    	    " --reason=" + ("Downloaded from " + cpio_p->name()).quote_shell();
    }

    //
    // read the project name from the archive,
    // and use it to default the project if not given
    //
    os_become_orig();
    nstring archive_name;
    ifp = cpio_p->child(archive_name);
    if (!ifp)
	missing_file(cpio_p, "etc/project-name");
    assert(!archive_name.empty());
    if (archive_name != "etc/project-name")
	wrong_file(ifp, "etc/project-name");
    nstring s;
    if (!ifp->one_line(s) || s.empty())
	ifp->fatal_error("short file");
    if (!project_name)
	project_name = str_copy(s.get_ref());
    delete ifp;
    os_become_undo();

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
    archive_name.clear();
    ifp = cpio_p->child(archive_name);
    if (!ifp)
	missing_file(cpio_p, "etc/change-set");
    assert(archive_name);
    if (archive_name == "etc/change-number")
    {
	s.clear();
	if (!ifp->one_line(s) || s.empty())
	    ifp->fatal_error("short file");
	long proposed_change_number = s.to_long();
	delete ifp;
	ifp = 0;
	os_become_undo();

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
	archive_name.clear();
	os_become_orig();
	ifp = cpio_p->child(archive_name);
	if (!ifp)
	    missing_file(cpio_p, "etc/change-set");
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
    if (archive_name != "etc/change-set")
	wrong_file(ifp, "etc/change-set");
    change_set = (cstate_ty *)parse_input(ifp, &cstate_type);
    ifp = 0;	// parse_input input_delete()ed it for us
    os_become_undo();

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
	cpio_p->fatal_error("bad change set");
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
	    cpio_p->fatal_error("bad change info");
    }

    //
    // We refuse to receive a change we already have because that can
    // undo changes.  If the local repository is the source of the
    // change set, then the patch will fail to apply and the full
    // source will be used, thus successive changes will be removed.
    // The -ignore-uuid option is here to handle UUID clash, if any.
    //
    nstring branch;
    nstring original_uuid;
    time_t original_ipass_when = 0;
    symtab<change_ty> local_inventory;
    bool include_branches = true;
    bool all_changes = false;
    bool ignore_original_uuid = true;
    change_functor_inventory_builder cf(include_branches, all_changes,
	ignore_original_uuid, pp, &local_inventory);
    project_inventory_walk(pp, cf);
    if (ignore_uuid <= 0)
    {
        if (change_set->uuid)
        {
            assert(universal_unique_identifier_valid(change_set->uuid));
            change_ty *c = local_inventory.query(change_set->uuid);
            if (c)
            {
                assert(c->cstate_data->uuid);
                assert(universal_unique_identifier_valid(c->cstate_data->uuid));
                assert(str_equal(change_set->uuid, c->cstate_data->uuid));

                //
                // run away, run away!
                //
                error_intl(0, i18n("change already present"));
                return;
            }
        }

        //
        // We now add to the local inventory also the original-UUIDs.
        //
	ignore_original_uuid = false;
        change_functor_inventory_builder cf2(include_branches, all_changes,
	    ignore_original_uuid, pp, &local_inventory);
        project_inventory_walk(pp, cf2);

        //
        // If the user has not specified a delta to copy file from, we
        // try to guess using the original-UUID attribute of the
        // change, if any.
        //
        if (delta.empty() && change_set->attribute)
        {
            //
            // Using the 'original-UUID' of the received change we look
            // for a completed change with a matching 'uuid'.
            //
            for (size_t i = 0; i < change_set->attribute->length; ++i)
            {
                attributes_ty *current = change_set->attribute->list[i];
                if (!current || !current->name || !current->value)
                    continue;
                if (0 != strcasecmp(current->name->str_text, ORIGINAL_UUID))
                    continue;
                if (!universal_unique_identifier_valid(current->value))
                    continue;
                change_ty *ancestor = local_inventory.query(current->value);
                if (!ancestor || !change_is_completed(ancestor))
                    continue;
                trace_string(current->value->str_text);

                //
                // We must keep track of the original-uuid and of the
                // timestamp of the change completion for later use in
                // the delta selection stage.
                //
                original_uuid = nstring(current->value);
                original_ipass_when =
                    change_when_get
                    (
                        ancestor,
                        cstate_history_what_integrate_pass
                    );

                //
                // This works even if the ancestor change is
                // in the trunk.
                //
		assert(change_delta_number_get(ancestor) > 0);
                delta =
	    	    nstring::format
		    (
			" --delta=%ld",
			change_delta_number_get(ancestor)
		    );
                trace_string(delta.c_str());

                if (ancestor->pp)
                    branch = nstring(project_version_short_get(ancestor->pp));
                if (!branch.empty())
                    branch = " --branch=" + branch;
                else
                    branch = " --trunk";
                trace_nstring(branch);
                break;
            }
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

    //
    // We handle the improvement cause differently because the
    // regression test exemption is false even if the project admin
    // selected default_change_exemption = true.
    //
    switch (cattr_data->cause)
    {
    case change_cause_internal_improvement:
    case change_cause_external_improvement:
        cattr_data->regression_test_exempt = change_set->regression_test_exempt;
        break;

    case change_cause_internal_bug:
    case change_cause_external_bug:
    case change_cause_internal_enhancement:
    case change_cause_external_enhancement:
    case change_cause_chain:
#ifndef DEBUG
    default:
#endif
        cattr_data->regression_test_exempt =
            (
                change_set->regression_test_exempt
            &&
                dflt->regression_test_exempt
            );
        break;
    }
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
    nstring trace_options(trace_args());
    dot = os_curdir();
    s =
	nstring::format
	(
	    "aegis --new-change %ld --project=%s --file=%s --verbose%s%s",
	    change_number,
	    project_name->str_text,
	    attribute_file_name->str_text,
	    reason.c_str(),
            trace_options.c_str()
	);
    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dot);
    os_unlink_errok(attribute_file_name);

    //
    // Begin development of the new change.
    //
    s =
	nstring::format
	(
	    "aegis --develop-begin %ld --project %s --verbose%s%s",
	    change_number,
	    project_name->str_text,
	    (devdir ? devdir->str_text : ""),
            trace_options.c_str()
	);
    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dot);
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
    // Adjust the file actions to ensure that renames are really
    // renames.  Historical versions of Aegis were not as fussy as now,
    // and you could get half a rename by aermu or aenfu of one half.
    //
    trace(("check that renames are complete\n"));
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty *src1 = change_set->src->list[j];
	if (!src1->move)
	    continue;
        trace(("%s \"%s\", move=\"%s\"\n", file_action_ename(src1->action),
               src1->file_name->str_text, src1->move->str_text));
	cstate_src_ty *src2 = find_src(change_set, src1->move);
	if (!src2)
	{
	    // Only half a move.
	    trace(("half a move\n"));
	    str_free(src1->move);
	    src1->move = 0;
	    continue;
	}

	switch (src1->action)
	{
	case file_action_create:
	    if (src2->action != file_action_remove)
	    {
		// Something really weird happened.
		trace(("not a move\n"));
		assert(0);
		str_free(src1->move);
		src1->move = 0;
	    }
	    break;

	case file_action_remove:
	    if (src2->action != file_action_create)
	    {
		// Something really weird happened.
		trace(("not a move\n"));
		assert(0);
		str_free(src1->move);
		src1->move = 0;
	    }
	    break;

	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
#ifdef DEBUG
	default:
#endif
	    assert(0);
	    //
            // None of these actions are supposed to have the move field
            // set.
	    //
	    trace(("not a move\n"));
	    str_free(src1->move);
	    src1->move = 0;
	    break;
	}
    }

    //
    // Adjust the file actions to reflect the current state of
    // the project.
    //
    trace(("adjust file actions\n"));
    bool need_to_test = false;
    could_have_a_trojan = 0;
    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;
	fstate_src_ty   *p_src_data;

        src_data = change_set->src->list[j];
        assert(src_data);
        assert(src_data->file_name);
	trace(("%d %s %s\n", j, src_data->file_name->str_text,
	    file_action_ename(src_data->action)));
	p_src_data =
	    project_file_find_by_meta2(pp, src_data, view_path_extreme);
	switch (src_data->action)
	{
	case file_action_remove:
	    //
	    // Removing a removed file would be an
	    // error, so take it out of the list completely.
	    //
	    trace(("remove\n"));
	    if (!p_src_data)
	    {
		trace(("no matching project file\n"));
		if (src_data->move)
		{
		    //
		    // does the target file exist?
		    //
		    trace(("which makes rename difficult\n"));
		    p_src_data =
			project_file_find
			(
			    pp,
			    src_data->move,
			    view_path_extreme
			);
		    cstate_src_ty *src2 = find_src(change_set, src_data->move);
		    assert(src2); // we already verified this
		    if (p_src_data)
		    {
			trace(("rewrite as modify\n"));
			src2->action = file_action_modify;
		    }
		    else
		    {
			trace(("rewrite as create\n"));
			src2->action = file_action_create;
		    }
		    str_free(src2->move);
		    src2->move = 0;

		    //
                    // We also want to ignore the remove half of the
                    // rename, so fall through...
		    //
		}

		//
                // It's tempting to fill the hole the removed element
                // leaves with the last list element and in doing so,
                // shortening the list.  But that would mean the order
                // of the files in the change set differs from the order
                // of the files in the cpio archive, but this would
                // create problems later.  We have to do it the long way
                // and shuffle everything down.
                //
		nuke_and_skip:
		trace(("nuke and skip\n"));
		for (size_t k = j + 1; k < change_set->src->length; ++k)
		    change_set->src->list[k - 1] = change_set->src->list[k];
		cstate_src_type.free(src_data);
		--j;
		change_set->src->length--;
		continue;
	    }

	    //
            // The view_path_extreme argument to project_find_file_by_
            // meta2 was supposed to take care of making removed files
            // disappear.
	    //
	    assert(p_src_data->action != file_action_remove);

            //
            // Take care of removing and renaming files which don't have
            // the same name in the project as in the incoming change set.
	    //
	    if (!str_equal(src_data->file_name, p_src_data->file_name))
	    {
		if (src_data->move)
		{
		    if (str_equal(src_data->move, p_src_data->file_name))
		    {
			//
                        // The file has already been renamed, so there
                        // is no need to remove it, but instead we turn
                        // the create half into a modify action.
			//
			trace(("rename becomes modify\n"));
			cstate_src_ty *src2 =
			    find_src(change_set, src_data->move);
			assert(src2);
			assert(src2->action == file_action_create);
			assert(src2->move);
			assert(str_equal(src2->move,src_data->file_name));
			str_free(src2->move);
			src2->move = 0;
			src2->action = file_action_modify;
			goto nuke_and_skip;
		    }

		    //
                    // We are about to update the file name in the
                    // incoming change set to match the file name in the
                    // project.  But first we must update the create
                    // half of the rename as well.
		    //
		    trace(("fix rename To half\n"));
		    cstate_src_ty *src2 =
			find_src(change_set, src_data->move);
		    assert(src2);
		    assert(src2->move);
		    assert(str_equal(src2->move, src_data->file_name));

		    str_free(src2->move);
		    src2->move = str_copy(p_src_data->file_name);
		}

		//
		// Update the file name in the incoming change set to
		// match the file name in the project.
		//
		trace(("file name changed\n"));
		str_free(src_data->file_name);
		src_data->file_name = str_copy(p_src_data->file_name);
	    }
	    break;

	case file_action_transparent:
	    trace(("transparent\n"));
	    assert(!src_data->move);
	    if (!p_src_data)
		goto nuke_and_skip;

	    //
	    // Do we need to change the name of the file?
	    //
	    if (!str_equal(src_data->file_name, p_src_data->file_name))
	    {
		trace(("file name changed\n"));
		str_free(src_data->file_name);
		src_data->file_name = str_copy(p_src_data->file_name);
	    }
            // FIXME: do we need to check that making it transparent is
            // even possible in this branch?!?
	    break;

	case file_action_create:
	    trace(("create\n"));
	    if (src_data->move)
	    {
		//
		// Has the rename already happened?
		//
		trace(("create half or rename\n"));
		if
		(
		    p_src_data
		&&
		    str_equal(src_data->file_name, p_src_data->file_name)
		)
		{
		    //
		    // Alter the file action to modify the file.
		    //
		    trace(("rename becomes modify\n"));
		    src_data->action = file_action_modify;

		    //
                    // Nuke the remove half of the rename.
		    //
                    // We can't just leave it for the remove half to
                    // clean up, because it may be after this point,
                    // and the clues are gone.  Plus, it would break
                    // the assertion that all renames have both halves
                    // present in the file list.
		    //
		    size_t m = 0;
		    for (m = 0; m < change_set->src->length; ++m)
		    {
			cstate_src_ty *src2 = change_set->src->list[m];
			if (str_equal(src_data->move, src2->file_name))
			{
			    assert(src2->action == file_action_remove);
                            cstate_src_type.free(src2);
                            break;
			}
		    }
		    assert(m < change_set->src->length);
		    for (size_t k = m + 1; k < change_set->src->length; ++k)
			change_set->src->list[k - 1] = change_set->src->list[k];
                    //
                    // We need to go back one step only if the remove
                    // half was before the current file.
                    //
                    if (m < j)
                        --j;
		    change_set->src->length--;
		}
		else
		{
		    //
		    // The rename hasn't happened yet.
		    // Do not mess with the file names.
		    //
		    trace(("rename is clean\n"));
		}
	    }
	    else
	    {
		//
                // The file exists in the project.  Alter the incoming
                // change set to modify the file.
		//
		if (p_src_data)
		{
		    //
                    // FIXME: we need to look for tests with the same
                    // name but different UUIDs, and automagically give
                    // them a new name, rather than simply ignoring the
                    // UUID.
		    //
		    trace(("create becomes modify\n"));
		    src_data->action = file_action_modify;

                    //
                    // If the name is different (this only happens when
                    // the incoming file has a UUID) make it match the
                    // project.
		    //
		    if (!str_equal(src_data->file_name, p_src_data->file_name))
		    {
			trace(("file name changed\n"));
			str_free(src_data->file_name);
			src_data->file_name = str_copy(p_src_data->file_name);
		    }
		}
	    }
	    break;

	case file_action_insulate:
	    assert(0);
	    // fall through...

	case file_action_modify:
	    trace(("modify\n"));
	    assert(!src_data->move);
	    if (!p_src_data)
	    {
                //
                // The named file doesn't exist in the project, to
                // convert the action into a create instead.
		//
		trace(("modify becomes create\n"));
		src_data->action = file_action_create;
	    }
	    else
	    {
                //
                // Make sure the action is a modify (it is supposed to
                // be impossible for an insulate action to get here, but
                // you never know).
		//
		src_data->action = file_action_modify;

		//
		// If the name is different (this only happens when
		// the incoming file has a UUID) make it match the
		// project.
		//
		if (!str_equal(src_data->file_name, p_src_data->file_name))
		{
		    trace(("file name changed\n"));
		    str_free(src_data->file_name);
		    src_data->file_name = str_copy(p_src_data->file_name);
		}
	    }
	    break;
	}

	//
	// Watch out for infection vectors.
	//
	trace(("check for trojans\n"));
	if (project_file_trojan_suspect(pp, src_data->file_name))
	    could_have_a_trojan = 1;
    }

    //
    // add the removed files to the change
    //
    trace(("look for removed files\n"));
    move_list_constructor(&files_moved);
    string_list_ty files_source;
    string_list_ty files_config;
    string_list_ty files_build;
    string_list_ty files_test_auto;
    string_list_ty files_test_manual;

    for (j = 0; j < change_set->src->length; ++j)
    {
	cstate_src_ty   *src_data;
        fstate_src_ty   *p_src_data;

	//
	// For now, we are only removing files.
	//
	src_data = change_set->src->list[j];
        assert(src_data);
        assert(src_data->file_name);
        p_src_data =
	    project_file_find_by_meta2(pp, src_data, view_path_extreme);
	switch (src_data->action)
	{
	case file_action_remove:
	    break;

	case file_action_modify:
            assert(p_src_data);
            if (p_src_data && p_src_data->usage != src_data->usage)
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
		    trace(("modify becomes create\n"));
                    src_data->action = file_action_create;

		    if (src_data->uuid)
		    {
			//
                        // Remove the UUID so that we don't try to set
                        // it later (because we do this, later, for all
                        // created files with UUIDs).
			//
			str_free(src_data->uuid);
			src_data->uuid = 0;
		    }
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
	    assert(src_data->action == file_action_remove);
	    move_list_append_remove
	    (
		&files_moved,
		src_data->file_name,
		src_data->move
	    );
	}
	else
	    files_source.push_back_unique(src_data->file_name);
    }
    if (files_source.nstrings)
    {
	s =
	    nstring::format
	    (
		"aegis --remove-file --project=%s --change=%ld%s --verbose",
		project_name->str_text,
		change_number,
                trace_options.c_str()
	    );
	os_xargs(s.get_ref(), &files_source, dd);
    }

    //
    // add the modified files to the change
    //
    trace(("look for modified files\n"));
    files_source.clear();
    symtab<string_list_ty> files_source_by_origin;
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
                //
                // We must initialize the origin with the information
                // from previous processing.
                //
                nstring origin = branch + delta;

                //
                // We have selected an origin delta using the
                // original-UUID attribute contained in the archive.
                // If we are receiving a branch that also contain the
                // entire_source we must check that the change set
                // that the UUID identify contains every single file.
                //
                if (!original_uuid.empty() && !branch.empty())
                {
                    change_ty *ancestor = local_inventory.query(original_uuid);
                    assert(ancestor);
                    fstate_src_ty *fsrc =
                        change_file_find
                        (
                            ancestor,
                            src_data->file_name,
                            view_path_first
                        );
                    //
                    // We reset the origin if the ancestor change does
                    // not contain the file.
                    //
                    if (!fsrc)
                        origin = "";
                }

                if (src_data->attribute)
                {
                    attributes_ty *edit_origin_UUID =
                        attributes_list_find
                        (
                            src_data->attribute,
                            EDIT_ORIGIN_UUID
                        );
                    if (edit_origin_UUID)
                    {
                        change_ty *ancestor =
                            local_inventory.query(edit_origin_UUID->value);

                        if (ancestor)
                        {
			    assert(change_delta_number_get(ancestor) > 0);
                            time_t ancestor_ipass_when =
                                change_when_get
                                (
                                    ancestor,
                                    cstate_history_what_integrate_pass
                                );

                            //
                            // We select the delta to merge with using
                            // the most recently integrated change set.
                            //
                            if (ancestor_ipass_when > original_ipass_when)
                            {
                                //
                                // We cannot reuse existing delta and
                                // branch variables, which used to
                                // contains data from previous processing,
                                // originl-UUID or options related,
                                // because this will cause wrong origin
                                // selection.
                                //
                                nstring ancestor_delta =
                                    nstring::format
                                    (
                                        " --delta=%ld",
                                        change_delta_number_get(ancestor)
                                    );
                                trace_string(ancestor_delta.c_str());

                                nstring ancestor_branch =
                                    nstring
                                    (
                                        project_version_short_get(ancestor->pp)
                                    );
                                if (!ancestor_branch.empty())
                                    ancestor_branch =
                                        " --branch=" + ancestor_branch;
                                else
                                    ancestor_branch = " --trunk";
                                trace_nstring(ancestor_branch);
                                origin = ancestor_branch + ancestor_delta;
                            }
                        }
                    }
                }

                //
                // We create a hash table with origin as the key and
                // with the list of files to be copied as the value.
                //
                string_list_ty *file_list =
                    files_source_by_origin.query(origin);
                if (!file_list)
                {
                    file_list = new string_list_ty;
                    files_source_by_origin.assign(origin, file_list);
                }
                file_list->push_back_unique(src_data->file_name);
                break;
	    }
	    break;

	case file_action_create:
	case file_action_remove:
	case file_action_transparent:
	    break;

	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    assert(0);
	    break;
	}

        //
        // FIXME: we need to look for tests with the same name but
        // different UUIDs, and automagically give them a new name,
        // rather than simply ignoring the UUID.
	//
    }
    uncopy = 0;

    //
    // We copy modified files from the baseline grouping them using
    // the origin.
    //
    if (!files_source_by_origin.empty())
    {
        uncopy = 1;
        nstring_list origin;

        files_source_by_origin.keys(origin);
        assert(!origin.empty());
        for (size_t c = 0; c < origin.size(); ++c)
        {
            string_list_ty *files_list =
                files_source_by_origin.query(origin[c]);
            assert(files_list->nstrings);
            s =
                nstring::format
                (
                    "aegis --copy-file --project=%s --change=%ld%s "
                    "--verbose%s",
                    project_name->str_text,
                    change_number,
                    trace_options.c_str(),
                    origin[c].c_str()
                );
            os_xargs(s.get_ref(), files_list, dd);
        }
    }

    //
    // add the new files to the change
    //
    trace(("look for created files\n"));
    files_source.clear();
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
	case file_action_transparent:
	    continue;

	case file_action_insulate:
#ifndef DEBUG
	default:
#endif
	    assert(0);
	    continue;
	}

	//
	// add it to the list
	//
	if (src_data->move)
	{
	    assert(src_data->action == file_action_create);
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
	    files_source.push_back_unique(src_data->file_name);
	    break;

	case file_usage_config:
	    files_config.push_back_unique(src_data->file_name);
	    break;

	case file_usage_build:
	    files_build.push_back_unique(src_data->file_name);
	    break;

	case file_usage_test:
	    files_test_auto.push_back_unique(src_data->file_name);
	    need_to_test = true;
	    break;

	case file_usage_manual_test:
	    files_test_manual.push_back_unique(src_data->file_name);
	    need_to_test = true;
	    break;
	}
    }

    if (files_build.nstrings)
    {
	s =
	    nstring::format
	    (
		"aegis --new-file --build --project=%s --change=%ld%s "
		    "--verbose --no-template",
		project_name->str_text,
		change_number,
                trace_options.c_str()
	    );
	os_xargs(s.get_ref(), &files_build, dd);
    }
    if (files_test_auto.nstrings)
    {
	s =
	    nstring::format
	    (
		"aegis --new-test --automatic --project=%s --change=%ld%s "
		    "--verbose --no-template",
		project_name->str_text,
		change_number,
                trace_options.c_str()
	    );
	os_xargs(s.get_ref(), &files_test_auto, dd);
    }
    if (files_test_manual.nstrings)
    {
	s =
	    nstring::format
	    (
		"aegis --new-test --manual --project=%s --change=%ld%s "
		    "--verbose --no-template",
		project_name->str_text,
		change_number,
                trace_options.c_str()
	    );
	os_xargs(s.get_ref(), &files_test_manual, dd);
    }
    if (files_source.nstrings)
    {
	s = nstring(THE_CONFIG_FILE_OLD);
	// FIXME: need to use the "is a config file" function
	if (files_source.member(s.get_ref()))
	{
	    //
	    // The project config file must be created in the last set
	    // of files created, so move it to the end of the list.
	    //
	    files_source.remove(s.get_ref());
	    files_config.push_back(s.get_ref());
	}

	s =
	    nstring::format
	    (
		"aegis --new-file --project=%s --change=%ld%s --verbose "
		    "--no-template",
		project_name->str_text,
		change_number,
                trace_options.c_str()
	    );
	os_xargs(s.get_ref(), &files_source, dd);
    }

    //
    // NOTE: Create project configuration files one last, in case the
    // incoming change set includes the first instance of the project
    // configuration file.  We don't want errors or inconsistencies in
    // the project configuration files preventing us from sucessfully
    // unpacking the change set.
    //
    if (files_config.nstrings)
    {
	s =
	    nstring::format
	    (
		"aegis --new-file --config --project=%s --change=%ld%s "
		    "--verbose --no-template",
		project_name->str_text,
		change_number,
                trace_options.c_str()
	    );
	os_xargs(s.get_ref(), &files_config, dd);
    }

    //
    // Now cope with files which moved.
    // They get a command each.
    //
    string_list_ty batch_moved;
    for (j = 0; j < files_moved.length; ++j)
    {
	move_ty         *mp;

	mp = files_moved.item + j;
	trace(("from=\"%s\"\n", mp->from ? mp->from->str_text : ""));
	trace(("remove=%d\n", mp->remove));
	trace(("to=\"%s\"\n", mp->to ? mp->to->str_text : ""));
	trace(("create=%d\n", mp->create));
	assert(mp->create);
	assert(mp->remove);
	assert(mp->from);
	assert(mp->to);
	batch_moved.push_back(mp->from);
	batch_moved.push_back(mp->to);
    }
    move_xargs(project_name, change_number, &batch_moved, dd, trace_options);
    move_list_destructor(&files_moved);

    //
    // now extract each file from the input
    //
    trace(("extract each file\n"));
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
	archive_name.clear();
	ifp = cpio_p->child(archive_name);
	if (!ifp)
	{
	    missing_file
	    (
		cpio_p,
		nstring::format("src/%s", src_data->file_name->str_text)
	    );
       	}
	assert(archive_name);
	need_whole_source = 1;
	s = nstring::format("patch/%s", src_data->file_name->str_text);
	if (archive_name == s)
	{
	    //
	    // We have a patch file, but we also know that a
	    // complete source follows.  We can apply the patch
	    // or discard it.  If we fail to apply it cleanly,
	    // we can always use the complete source which follows.
	    //
	    patch_list_ty *plp = patch_read(ifp, 0);
	    delete ifp;
	    ifp = 0;

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
                    // The patch does not contain any lines, so this is
                    // a "clean" rename without changes to the new file.
                    // Thus, the whole source is not needed.
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
		    {
			//
                        // Applying the patch succeeded, so we don't
                        // need to extract the whole source.
			//
			need_whole_source = 0;
		    }
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
	    archive_name.clear();
	    ifp = cpio_p->child(archive_name);
	    if (!ifp)
	    {
		missing_file
		(
	    	    cpio_p,
	    	    nstring::format("src/%s", src_data->file_name->str_text)
		);
	    }
	    assert(archive_name);
	}
	s = nstring::format("src/%s", src_data->file_name->str_text);
	if (archive_name != s)
	    wrong_file(ifp, s);
	if (need_whole_source)
	    ofp = output_file_binary_open(src_data->file_name);
	else
	    ofp = new output_bit_bucket();
	input_to_output(ifp, ofp);
	delete ofp;
	delete ifp;
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
            //
            // We remove the edit-origin-UUID from the file's
            // attributes list. It may be not correct for the local
            // repository.
            //
            attributes_list_remove(fattr_data->attribute, EDIT_ORIGIN_UUID);
	    fattr_write_file(attribute_file_name, fattr_data, 0);
	    fattr_type.free(fattr_data);
	    s2 = str_quote_shell(src_data->file_name);
	    s =
		nstring::format
		(
		    "aegis --file-attributes --change=%ld --project=%s "
			"--file=%s --verbose %s --base-rel%s",
		    change_number,
		    project_name->str_text,
		    attribute_file_name->str_text,
		    s2->str_text,
                    trace_options.c_str()
		);
	    str_free(s2);
	    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	    os_unlink_errok(attribute_file_name);
	}

	switch (src_data->action)
	{
	case file_action_create:
	    if (src_data->uuid && !src_data->move)
	    {
		//
                // The incoming change set specifies a UUID for this
                // file, run "aefa -uuid" to set it.
		//
		string_ty *qfn = str_quote_shell(src_data->file_name);
		s =
		    nstring::format
		    (
			"aegis --file-attributes --uuid %s --change=%ld "
			    "--project=%s --verbose %s --base-rel%s",
			src_data->uuid->str_text,
			change_number,
			project_name->str_text,
			qfn->str_text,
			trace_options.c_str()
		    );
		str_free(qfn);
		os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	    }
	    break;

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
	cstate_src_ty   *src_data;

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
    archive_name.clear();
    ifp = cpio_p->child(archive_name);
    if (ifp)
	cpio_p->fatal_error("archive too long");
    delete cpio_p;
    os_become_undo();

    //
    // Now that all the files have been unpacked,
    // set the change's UUID.
    //
    // It is vaguely possible you have already downloaded this change
    // before, so we don't complain (OS_EXEC_FLAG_ERROK) if the command
    // fails.
    //
    // We intentionally set the UUID before the aecpu -unch.  The aecpu
    // -unch will clear the UUID if the change's file inventory changes,
    // because it is no longer the same change set.
    //
    if (change_set->uuid)
    {
	string_ty       *quoted_uuid;

	quoted_uuid = str_quote_shell(change_set->uuid);
	s =
	    nstring::format
	    (
		"aegis --change-attr --uuid %s -change=%ld --project=%s%s",
		quoted_uuid->str_text,
		change_number,
		project_name->str_text,
                trace_options.c_str()
	    );
	str_free(quoted_uuid);
	os_become_orig();
	os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT | OS_EXEC_FLAG_ERROK, dd);
	os_become_undo();
    }

    //
    // If we are receiving a change set with an UUID, and the user has
    // not used the --ignore-uuid option we avoid the uncopy files so
    // the UUID is preserved and we don't need a new one.  This should
    // reduce the UUID proliferation and some related side effects.
    //
    if (change_set->uuid && ignore_uuid <= 0)
        uncopy = 0;

    //
    // Un-copy any files which did not change.
    //
    // The idea is, if there are no files left, there is nothing
    // for this change to do, so cancel it.
    //
    // We intentionally set the UUID before the aecpu -unch.  The aecpu
    // -unch will clear the UUID if the change's file inventory changes,
    // because it is no longer the same change set.
    //
    if (uncopy)
    {
	s =
	    nstring::format
	    (
		"aegis --copy-file-undo --unchanged --change=%ld --project=%s "
		    "--verbose%s",
		change_number,
		project_name->str_text,
                trace_options.c_str()
	    );
	os_become_orig();
	os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();

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
		nstring::format
		(
		    "aegis --develop-begin-undo --change=%ld --project=%s "
			"--verbose%s",
		    change_number,
		    project_name->str_text,
                    trace_options.c_str()
		);
	    os_become_orig();
	    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dot);

	    //
	    // cancel the change
	    //
	    s =
		nstring::format
		(
		    "aegis --new-change-undo --change=%ld --project=%s "
			"--verbose%s",
		    change_number,
		    project_name->str_text,
                    trace_options.c_str()
		);
	    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dot);
	    os_become_undo();

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
	    nstring::format
	    (
		"aegis --change-attr --fix-arch --change=%ld --project=%s%s",
		change_number,
		project_name->str_text,
                trace_options.c_str()
	    );
	os_become_orig();
	os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	return;
    }

    //
    // now merge the change
    //
    s =
	nstring::format
	(
	    "aegis --diff --only-merge --change=%ld --project=%s --verbose",
	    change_number,
	    project_name->str_text
        );
    os_become_orig();
    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();

    //
    // now diff the change
    //
    s =
	nstring::format
	(
	    "aegis --diff --no-merge --change=%ld --project=%s --verbose%s",
	    change_number,
	    project_name->str_text,
            trace_options.c_str()
	);
    os_become_orig();
    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();

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
	nstring::format
	(
	    "aegis --build --change=%ld --project=%s --verbose%s",
	    change_number,
	    project_name->str_text,
            trace_options.c_str()
	);
    os_become_orig();
    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();

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
	    nstring::format
	    (
		"aegis --test --change=%ld --project=%s --verbose%s",
		change_number,
		project_name->str_text,
                trace_options.c_str()
	    );
	os_become_orig();
	os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
    }
    if (need_to_test && !cstate_data->test_baseline_exempt)
    {
	s =
	    nstring::format
	    (
		"aegis --test --baseline --change=%ld --project=%s "
                "--verbose%s",
		change_number,
		project_name->str_text,
                trace_options.c_str()
	    );
	os_become_orig();
	os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
    }

    // always to a regession test?
    if (!cstate_data->regression_test_exempt)
    {
	s =
	    nstring::format
	    (
		"aegis --test --regression --change=%ld --project=%s "
                "--verbose%s",
		change_number,
		project_name->str_text,
                trace_options.c_str()
	    );
	os_become_orig();
	os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
    }

    change_free(cp);
    cp = 0;
    project_free(pp);
    pp = 0;

    //
    // end development (if we got this far!)
    //
    s =
	nstring::format
	(
	    "aegis --develop-end --change=%ld --project=%s --verbose%s",
	    change_number,
	    project_name->str_text,
            trace_options.c_str()
	);
    os_become_orig();
    os_execute(s.get_ref(), OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();

    // verbose success message here?
}
