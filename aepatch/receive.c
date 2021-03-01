/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate receives
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <arglex3.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <cattr.h>
#include <change.h>
#include <change/attributes.h>
#include <change/file.h>
#include <error.h>
#include <help.h>
#include <os.h>
#include <patch/file.h>
#include <pconf.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/file/trojan.h>
#include <project/history.h>
#include <receive.h>
#include <slurp.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <zero.h>


static void
usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf(stderr, "Usage: %s --receive [ <option>... ]\n", progname);
    fprintf(stderr, "       %s --help\n", progname);
    exit(1);
}


static string_ty *
is_a_path_prefix(string_ty *haystack, string_ty *needle)
{
    if
    (
	haystack->str_length > needle->str_length
    &&
	haystack->str_text[needle->str_length] == '/'
    &&
	0 == memcmp(haystack->str_text, needle->str_text, needle->str_length)
    )
    {
	return
	    str_n_from_c
	    (
		haystack->str_text + needle->str_length + 1,
		haystack->str_length - needle->str_length - 1
	    );
    }
    return 0;
}


static string_ty *path_prefix_add;
static string_list_ty path_prefix_remove;


static void
mangle_file_names(patch_list_ty *plp, project_ty *pp)
{
    typedef struct cmp_t cmp_t;
    struct cmp_t
    {
	int		npaths;
	int		idx;
    };

    size_t	    j;
    cmp_t	    best;
    patch_ty	    *p;
    string_ty	    *s;
    int		    npaths;
    size_t	    idx;
    char	    *cp;
    string_ty	    *dev_null;

    /*
     * First we chew over the filenames according to the path prefix
     * options on the command line.
     *
     * First remove any path prefixes we have been asked to remove.
     * Second add a path prefix if we have been asked to.
     */
    trace(("mangle_file_names()\n{\n"));
    for (j = 0; j < plp->length; ++j)
    {
	p = plp->item[j];
	for (idx = 0; idx < p->name.nstrings; ++idx)
	{
	    size_t          k;

	    for (k = 0; k < path_prefix_remove.nstrings; ++k)
	    {
		s =
	    	    is_a_path_prefix
		    (
			p->name.string[idx],
			path_prefix_remove.string[k]
		    );
		if (s)
		{
		    str_free(p->name.string[idx]);
		    p->name.string[idx] = s;
		}
	    }
	    if (path_prefix_add)
	    {
		s = os_path_cat(path_prefix_add, p->name.string[idx]);
		str_free(p->name.string[idx]);
		p->name.string[idx] = s;
	    }
	}
    }

    /*
     * Look for the name with the fewest removed leading path
     * compenents that produces a file name which exists in the
     * project.
     */
    dev_null = str_from_c("/dev/null");
    best.npaths = 32767;
    best.idx = 32767;
    for (j = 0; j < plp->length; ++j)
    {
	p = plp->item[j];
	for (idx = 0; idx < p->name.nstrings; ++idx)
	{
	    npaths = 0;
	    if (str_equal(dev_null, p->name.string[idx]))
		continue;
	    cp = p->name.string[idx]->str_text;
	    for (;;)
	    {
		s = str_from_c(cp);
		if (project_file_find(pp, s, view_path_extreme))
		{
		    if
		    (
			npaths < best.npaths
		    ||
			(npaths == best.npaths && idx < best.idx)
		    )
		    {
			best.npaths = npaths;
			best.idx = idx;
			break;
		    }
		}
		cp = strchr(cp, '/');
		if (!cp)
		    break;
		++cp;
		if (!*cp)
		    break;
		++npaths;
	    }
	}
    }
    str_free(dev_null);
    if (best.npaths == 32767)
    {
	best.npaths = 0;
	best.idx = 0;
    }

    /*
     * Now adjust the file names, using the path information.
     */
    for (j = 0; j < plp->length; ++j)
    {
	char		*cp2;

	/*
	 * Rip the right number of path elements from the
	 * "best" name.	 Note that we have to cope with the
	 * number of names in the patch being inconsistent.
	 */
	p = plp->item[j];
	s = p->name.string[0];
	if (best.idx < p->name.nstrings)
	    s = p->name.string[best.idx];
	cp = s->str_text;
	for (npaths = best.npaths; npaths > 0; --npaths)
	{
	    cp2 = strchr(cp, '/');
	    if (!cp2)
		break;
	    cp = cp2 + 1;
	}
	trace(("%s -> %s\n", p->name.string[0]->str_text, cp));

	/*
	 * Insert the reconstructed name into the front of the
	 * list of names.
	 */
	s = str_from_c(cp);
	string_list_prepend(&p->name, s);
	str_free(s);
    }
    trace(("}\n"));
}


static long
number_of_files(string_ty *project_name, long change_number)
{
    project_ty	    *pp;
    change_ty	    *cp;
    long	    result;

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
receive(void)
{
    string_ty	    *ifn;
    string_ty	    *s;
    patch_list_ty   *plp;
    size_t	    j;
    string_ty	    *project_name;
    long	    change_number;
    project_ty	    *pp;
    change_ty	    *cp;
    string_ty	    *attribute_file_name;
    cattr_ty	    *cattr_data;
    cattr_ty	    *dflt;
    string_ty	    *dot;
    string_ty	    *devdir;
    pconf_ty	    *pconf_data;
    string_ty	    *dd;
    int		    need_to_test;
    int		    could_have_a_trojan;
    const char      *delta;
    string_list_ty  files_source;
    string_list_ty  files_config;
    string_list_ty  files_build;
    string_list_ty  files_test_auto;
    string_list_ty  files_test_manual;
    int		    uncopy;
    int		    config_seen;
    int		    trojan;

    trace(("receive()\n{\n"));
    project_name = 0;
    change_number = 0;
    ifn = 0;
    devdir = 0;
    delta = 0;
    trojan = -1;
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
		/* NOTREACHED */

	    case arglex_token_stdio:
		ifn = str_from_c("");
		break;

	    case arglex_token_string:
		ifn = str_from_c(arglex_value.alv_string);
		break;
	    }
	    break;

	case arglex_token_directory:
	    if (devdir)
	    {
		duplicate_option(usage);
		/* NOTREACHED */
	    }
	    if (arglex() != arglex_token_string)
	    {
		option_needs_dir(arglex_token_directory, usage);
		/* NOTREACHED */
	    }
	    devdir = str_format(" --directory %s", arglex_value.alv_string);
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
		/*NOTREACHED*/

	    case arglex_token_number:
	    case arglex_token_string:
		delta = arglex_value.alv_string;
		break;
	    }
	    break;

	case arglex_token_path_prefix_add:
	    if (path_prefix_add)
		duplicate_option(usage);
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_path_prefix_add, usage);
	    path_prefix_add = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_path_prefix_remove:
	    if (arglex() != arglex_token_string)
		option_needs_file(arglex_token_path_prefix_add, usage);
	    s = str_from_c(arglex_value.alv_string);
	    string_list_append_unique(&path_prefix_remove, s);
	    str_free(s);
	    break;
	}
	arglex();
    }

    /*
     * read the input
     */
    plp = patch_slurp(ifn);
    assert(plp);

    if (!project_name)
	project_name = plp->project_name;
    if (!project_name)
	project_name = user_default_project();

    /*
     * locate project data
     *	    (Even of we don't use it, this confirms it is a valid
     *	    project name.)
     */
    pp = project_alloc(project_name);
    project_bind_existing(pp);

    /*
     * Search the names in the patch, trying to figure out how much
     * path prefix to throw away.  When this is done, name.string[0]
     * is the name we will use for the files.
     */
    mangle_file_names(plp, pp);

    /*
     * default the change number
     *
     * Note that the change number in the patch is advisory only, if we
     * can't get it, just use the next available.
     */
    if (!change_number)
    {
	if
	(
	    plp->change_number
	&&
	    !project_change_number_in_use(pp, plp->change_number)
	)
	    change_number = plp->change_number;
	else
	    change_number = project_next_change_number(pp, 1);
    }

    /*
     * construct change attributes from the patch
     */
    os_become_orig();
    attribute_file_name = os_edit_filename(0);
    undo_unlink_errok(attribute_file_name);
    cattr_data = cattr_type.alloc();
    if (plp->brief_description)
	cattr_data->brief_description = str_copy(plp->brief_description);
    else
	cattr_data->brief_description = str_from_c("none");
    if (plp->description)
	cattr_data->description = str_copy(plp->description);
    else
	cattr_data->description = str_from_c("none");
    cattr_data->cause = change_cause_external_bug;
    dflt = cattr_type.alloc();
    dflt->cause = cattr_data->cause;
    os_become_undo();
    pconf_data = project_pconf_get(pp);
    change_attributes_default(dflt, pp, pconf_data);
    os_become_orig();
    cattr_data->test_exempt = dflt->test_exempt;
    cattr_data->test_baseline_exempt = dflt->test_baseline_exempt;
    cattr_data->regression_test_exempt = dflt->regression_test_exempt;
    cattr_type.free(dflt);
    cattr_write_file(attribute_file_name, cattr_data, 0);
    cattr_type.free(cattr_data);
    project_free(pp);
    pp = 0;

    /*
     * create the new change
     */
    dot = os_curdir();
    s =
	str_format
	(
	    "aegis --new-change %ld --project=%S --file=%S --verbose",
	    change_number,
	    project_name,
	    attribute_file_name
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_unlink_errok(attribute_file_name);
    str_free(attribute_file_name);

    /*
     * Begin development of the new change.
     */
    s =
	str_format
	(
	    "aegis --develop-begin %ld --project %S --verbose%s",
	    change_number,
	    project_name,
	    (devdir ? devdir->str_text : "")
	);
    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
    str_free(s);
    os_become_undo();

    /*
     * Change to the development directory, so that we can use
     * relative filenames.  It makes things easier to read.
     */
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    dd = change_development_directory_get(cp, 0);
    dd = str_copy(dd); /* will vanish when change_free(); */
    change_free(cp);
    cp = 0;

    os_chdir(dd);

    /*
     * Adjust the file actions to reflect the current state of
     * the project.
     */
    need_to_test = 0;
    could_have_a_trojan = 0;
    for (j = 0; j < plp->length; ++j)
    {
	patch_ty	*p;
	fstate_src_ty   *p_src_data;

	p = plp->item[j];
	assert(p->name.nstrings>=2);
	p_src_data =
	    project_file_find(pp, p->name.string[0], view_path_extreme);
	if (!p_src_data)
	{
	    switch (p->action)
	    {
	    case file_action_remove:
		/*
		 * Removing a removed file would be an
		 * error.  Get rid of it.
		 */
		plp->item[j] = plp->item[plp->length - 1];
		plp->length--;
		--j;
		patch_delete(p);
		continue;

	    case file_action_insulate:
	    case file_action_transparent:
		assert(0);

	    case file_action_create:
		break;

	    case file_action_modify:
#ifndef DEBUG
	    default:
#endif
		p->action = file_action_create;
		break;
	    }
	}
	else
	{
	    switch (p->action)
	    {
	    case file_action_remove:
		break;

	    case file_action_modify:
		break;

	    case file_action_create:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		p->action = file_action_modify;
		break;
	    }
	}
	if (project_file_trojan_suspect(pp, p->name.string[0]))
	    could_have_a_trojan = 1;
    }
    project_free(pp);
    pp = 0;

    /*
     * add the modified files to the change
     */
    string_list_constructor(&files_source);
    string_list_constructor(&files_test_auto);
    string_list_constructor(&files_test_manual);
    for (j = 0; j < plp->length; ++j)
    {
	patch_ty	*p;

	/*
	 * For now, we are only copying files.
	 */
	p = plp->item[j];
	assert(p->name.nstrings>=2);
	switch (p->action)
	{
	case file_action_modify:
	    break;

	case file_action_create:
	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;
	}
	switch (p->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	case file_usage_config:
	    break;

	case file_usage_test:
	case file_usage_manual_test:
	    need_to_test = 1;
	    break;
	}

	/*
	 * add it to the list
	 */
	string_list_append_unique(&files_source, p->name.string[0]);
    }
    uncopy = 0;
    if (files_source.nstrings)
    {
	string_ty	*delopt;

	delopt = 0;
	if (delta)
	{
	    delopt = str_from_c(delta);
	    s = str_quote_shell(delopt);
	    str_free(delopt);
	    delopt = str_format(" --delta=%S", s);
	    str_free(s);
	}
	uncopy = 1;
	s =
	    str_format
	    (
		"aegis --copy-file --project=%S --change=%ld --verbose%s",
		project_name,
		change_number,
		(delopt ? delopt->str_text : "")
	    );
	if (delopt)
	    str_free(delopt);
	os_xargs(s, &files_source, dd);
	str_free(s);
    }
    string_list_destructor(&files_source);

    /*
     * add the removed files to the change
     */
    for (j = 0; j < plp->length; ++j)
    {
	patch_ty	*p;

	/*
	 * For now, we are only removing files.
	 */
	p = plp->item[j];
	assert(p->name.nstrings>=2);
	switch (p->action)
	{
	case file_action_remove:
	    break;

	case file_action_create:
	case file_action_modify:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    continue;
	}

	/*
	 * add it to the list
	 */
	string_list_append_unique(&files_source, p->name.string[0]);
    }
    if (files_source.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --remove-file --project=%S --change=%ld --verbose",
		project_name,
		change_number
	    );
	os_xargs(s, &files_source, dd);
	str_free(s);
    }
    string_list_destructor(&files_source);

    /*
     * add the new files to the change
     */
    string_list_constructor(&files_config);
    string_list_constructor(&files_build);
    need_to_test = 0;
    for (j = 0; j < plp->length; ++j)
    {
	patch_ty	*p;

	/*
	 * for now, we are only dealing with create
	 */
	p = plp->item[j];
	assert(p->name.nstrings>=2);
	switch (p->action)
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

	/*
	 * add it to the list
	 */
	switch (p->usage)
	{
	case file_usage_source:
	    string_list_append_unique(&files_source, p->name.string[0]);
	    break;

	case file_usage_config:
	    string_list_append_unique(&files_config, p->name.string[0]);
	    break;

	case file_usage_build:
	    string_list_append_unique(&files_build, p->name.string[0]);
	    break;

	case file_usage_test:
	    string_list_append_unique(&files_test_auto, p->name.string[0]);
	    need_to_test = 1;
	    break;

	case file_usage_manual_test:
	    string_list_append_unique(&files_test_manual, p->name.string[0]);
	    need_to_test = 1;
	    break;
	}
    }

    if (files_test_auto.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-test --automatic --project=%S --change=%ld "
		    "--verbose --no-template",
		project_name,
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
		"aegis --new-test --manual --project=%S --change=%ld --verbose "
		    "--no-template",
		project_name,
		change_number
	    );
	os_xargs(s, &files_test_manual, dd);
	str_free(s);
    }

    /*
     * NOTE: do this one last, in case it includes the first instance
     * of the project config file.
     */
    if (files_source.nstrings)
    {
	s = str_from_c(THE_CONFIG_FILE);
	if (string_list_member(&files_source, s))
	{
	    /*
	     * The project config file must be created in the last set
	     * of files created, so move it to the end of the list.
	     */
	    string_list_remove(&files_source, s);
	    string_list_append(&files_source, s);
	}
	str_free(s);

	s =
	    str_format
	    (
		"aegis --new-file --project=%S --change=%ld --verbose "
		    "--no-template",
		project_name,
		change_number
	    );
	os_xargs(s, &files_source, dd);
	str_free(s);
    }
    if (files_build.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-file --build --project=%S --change=%ld --verbose "
		    "--no-template",
		project_name,
		change_number
	    );
	os_xargs(s, &files_build, dd);
	str_free(s);
    }
    if (files_config.nstrings)
    {
	s =
	    str_format
	    (
		"aegis --new-file --config --project=%S --change=%ld --verbose "
		    "--no-template",
		project_name,
		change_number
	    );
	os_xargs(s, &files_config, dd);
	str_free(s);
    }
    string_list_destructor(&files_source);
    string_list_destructor(&files_config);
    string_list_destructor(&files_build);
    string_list_destructor(&files_test_auto);
    string_list_destructor(&files_test_manual);

    /*
     * now extract each file from the input
     */
    config_seen = 0;
    pp = project_alloc(project_name);
    project_bind_existing(pp);
    cp = change_alloc(pp, change_number);
    change_bind_existing(cp);
    for (j = 0; j < plp->length; ++j)
    {
	patch_ty	*p;
	string_ty	*orig;

	/* verbose progress message here? */
	p = plp->item[j];
	switch (p->action)
	{
	case file_action_create:
	case file_action_modify:
	    break;

	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
	    continue;
	}
	switch (p->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	    if (change_file_is_config(cp, p->name.string[0]))
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
	assert(p->name.nstrings>=2);
	trace(("%s\n", p->name.string[0]->str_text));

	/*
	 * Recall that, somewhere above, we may have messed
	 * with the `action' field, so we have to look at the
	 * patch itself, and reconstruct whether it is creating
	 * new content.
	 */
	if
	(
	    p->actions.length == 1
	&&
	    p->actions.item[0]->before.length == 0
	&&
	    p->actions.item[0]->before.start_line_number == 0
	)
	    p->action = file_action_create;

	/*
	 * Apply the patch.
	 */
	switch (p->action)
	{
	case file_action_create:
	    os_become_orig();
	    patch_apply(p, (string_ty *)0, p->name.string[0]);
	    os_become_undo();
	    break;

	case file_action_modify:
	    /*
	     * This is the normal case: modify an existing file.
	     *
	     * The input file (to which the patch is applied) may
	     * be found in the baseline.
	     */
	    orig = project_file_path(pp, p->name.string[0]);
	    os_become_orig();
	    patch_apply(p, orig, p->name.string[0]);
	    os_become_undo();
	    str_free(orig);
	    break;

	case file_action_remove:
	    break;

	case file_action_insulate:
	case file_action_transparent:
	    assert(0);
	    break;
	}
    }
    change_free(cp);
    cp = 0;
    project_free(pp);
    pp = 0;

    /*
     * Un-copy any files which did not change.
     *
     * The idea is, if there are no files left, there is nothing
     * for this change to do, so cancel it.
     */
    if (uncopy)
    {
	s =
	    str_format
	    (
       "aegis --copy-file-undo --unchanged --change=%ld --project=%S --verbose",
		change_number,
		project_name
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);

	/*
	 * If there are no files left, we already have this change.
	 */
	if (number_of_files(project_name, change_number) == 0)
	{
	    /*
	     * get out of there
	     */
	    os_chdir(dot);

	    /*
	     * stop developing the change
	     */
	    s =
		str_format
		(
	       "aegis --develop-begin-undo --change=%ld --project=%S --verbose",
		    change_number,
		    project_name
		);
	    os_become_orig();
	    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	    str_free(s);

	    /*
	     * cancel the change
	     */
	    s =
		str_format
		(
		  "aegis --new-change-undo --change=%ld --project=%S --verbose",
		    change_number,
		    project_name
		);
	    os_execute(s, OS_EXEC_FLAG_INPUT, dot);
	    os_become_undo();
	    str_free(s);

	    /*
	     * run away, run away!
	     */
	    error_intl(0, i18n("change already present"));
	    return;
	}
    }

    /*
     * If the change could have a trojan horse in it, stop here with
     * a warning.  The user needs to look at it and check.
     */
    if (trojan > 0)
	could_have_a_trojan = 1;
    else if (trojan == 0)
    {
	error_intl(0, i18n("warning: potential trojan, proceeding anyway"));
	could_have_a_trojan = 0;
	config_seen = 0;
    }

    /*
     * If the change could have a trojan horse in the project config
     * file, stop here with a warning.	Don't even difference the
     * change, because the trojan could be embedded in the diff
     * command.	 The user needs to look at it and check.
     *
     * FIX ME: what if the aecpu got rid of it?
     */
    if (config_seen)
    {
	error_intl
	(
	    0,
	 i18n("warning: potential trojan, review before completing development")
	);
	return;
    }

    /*
     * now diff the change
     */
    s =
	str_format
	(
	    "aegis --diff --no-merge --change=%ld --project=%S --verbose",
	    change_number,
	    project_name
	);
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    /*
     * If the change could have a trojan horse in it, stop here with
     * a warning.  The user needs to look at it and check.
     */
    if (could_have_a_trojan)
    {
	error_intl
	(
	    0,
	 i18n("warning: potential trojan, review before completing development")
	);
	return;
    }

    /*
     * now build the change
     */
    s =
	str_format
	(
	    "aegis --build --change=%ld --project=%S --verbose",
	    change_number,
	    project_name
	);
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    /*
     * now test the change
     */
    if (need_to_test)
    {
	s =
	    str_format
	    (
		"aegis --test --change=%ld --project=%S --verbose",
		change_number,
		project_name
	    );
	os_become_orig();
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	str_free(s);

	s =
	    str_format
	    (
		"aegis --test --baseline --change=%ld --project=%S --verbose",
		change_number,
		project_name
	    );
	os_execute(s, OS_EXEC_FLAG_INPUT, dd);
	os_become_undo();
	str_free(s);
    }

    /* always do a regession test? */

    /*
     * end development (if we got this far!)
     */
    s =
	str_format
	(
	    "aegis --develop-end --change=%ld --project=%S --verbose",
	    change_number,
	    project_name
	);
    os_become_orig();
    os_execute(s, OS_EXEC_FLAG_INPUT, dd);
    os_become_undo();
    str_free(s);

    /* verbose success message here? */
    trace(("}\n"));
}
