/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2003 Peter Miller;
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
 * MANIFEST: functions to implement test
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <ael/change/files.h>
#include <aet.h>
#include <arglex2.h>
#include <arglex/change.h>
#include <arglex/project.h>
#include <change.h>
#include <change/branch.h>
#include <change/file.h>
#include <change/test/run_list.h>
#include <commit.h>
#include <error.h>
#include <help.h>
#include <lock.h>
#include <log.h>
#include <mem.h>
#include <now.h>
#include <os.h>
#include <progname.h>
#include <project.h>
#include <project/file.h>
#include <project/history.h>
#include <r250.h>
#include <str_list.h>
#include <sub.h>
#include <symtab.h>
#include <trace.h>
#include <user.h>


static void
test_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Test [ <filename>... ][ <option>... ]\n",
	progname
    );
    fprintf(stderr, "       %s -Test -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Test -Help\n", progname);
    quit(1);
}


static void
test_help(void)
{
    help("aet", test_usage);
}


static void
test_list(void)
{
    string_ty	    *project_name;
    long	    change_number;

    trace(("test_list()\n{\n"));
    project_name = 0;
    change_number = 0;
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(test_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    /* fall through... */

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, test_usage);
	    continue;

	case arglex_token_project:
	    arglex();
	    /* fall through... */

	case arglex_token_string:
	    arglex_parse_project(&project_name, test_usage);
	    continue;
	}
	arglex();
    }
    list_change_files(project_name, change_number, 0);
    if (project_name)
	str_free(project_name);
    trace(("}\n"));
}


typedef struct limit_suggestions_ty limit_suggestions_ty;
struct limit_suggestions_ty
{
    string_ty	    *filename;
    double	    count;
};


static int
limit_suggestions_cmp(const void *va, const void *vb)
{
    const limit_suggestions_ty *a;
    const limit_suggestions_ty *b;

    a = va;
    b = vb;

    /*
     * Note reversed polarity:
     * we want to sort largest to smallest.
     */
    if (a->count < b->count)
	return 1;
    if (a->count > b->count)
	return -1;
    return 0;
}


static void
limit_suggestions(change_ty *cp, string_list_ty *flp, int want,
    int noise_percent)
{
    project_ty	    *pp;
    limit_suggestions_ty *item;
    size_t	    nitems;
    limit_suggestions_ty *lp;
    symtab_ty	    *stp;
    size_t	    j;
    double	    frac;
    double	    count_max;

    /*
     * do nothing if at all possible
     */
    if (want > 0 && flp->nstrings < want)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number1", (long)flp->nstrings);
	sub_var_optional(scp, "Number1");
	sub_var_set_long(scp, "Number2", (long)want);
	sub_var_optional(scp, "Number2");
	change_verbose(cp, scp, i18n("few candidate tests"));
	sub_context_delete(scp);
    }
    if (want < 0)
	want = 10;
    if (flp->nstrings <= want)
	return;

    /*
     * Build the list of candidate files, and initialize their
     * counts.
     */
    pp = cp->pp;
    nitems = flp->nstrings;
    item = mem_alloc(nitems * sizeof(item[0]));
    stp = symtab_alloc(nitems);
    for (j = 0; j < nitems; ++j)
    {
	lp = item + j;
	lp->filename = str_copy(flp->string[j]);
	lp->count = 0;
	symtab_assign(stp, lp->filename, lp);
    }
    string_list_destructor(flp);
    count_max = 0;

    /*
     * Examine each change source file looking for test correlations.
     * Candidates are limited to the list given in the arguments.
     *
     * The count is weighted.  Source files which are correlated to
     * many files result in a low weighting, source files which are
     * correlated to few tests recieve a high weighting.  This
     * biases the test selection to those tests specifically aimed
     * at the files in this change.
     */
    for (j = 0;; ++j)
    {
	fstate_src_ty   *c_src_data;
	fstate_src_ty   *p_src_data;
	size_t		k;

	c_src_data = change_file_nth(cp, j);
	if (!c_src_data)
	    break;
	switch (c_src_data->usage)
	{
	case file_usage_source:
	case file_usage_config:
	    break;

	case file_usage_test:
	case file_usage_manual_test:
	    /*
	     * Correlations with other tests, while interesting,
	     * aren't useful.
	     */
	    continue;

	case file_usage_build:
#ifndef DEBUG
	default:
#endif
	    continue;
	}
	p_src_data =
	    project_file_find(pp, c_src_data->file_name, view_path_extreme);
	if (!p_src_data)
	    continue;
	if (!p_src_data->test || !p_src_data->test->length)
	    continue;
	frac = 1. / p_src_data->test->length;
	for (k = 0; k < p_src_data->test->length; ++k)
	{
	    lp = symtab_query(stp, p_src_data->test->list[k]);
	    if (lp)
	    {
		lp->count += frac;
		if (lp->count > count_max)
		    count_max = lp->count;
	    }
	}
    }
    symtab_free(stp);

    /*
     * Add noise to the counts so that unlikely things are tested
     * once in a while, and so that the same tests are not run over
     * and over.  This also varies the tests run when there are a
     * large number with the same (or very similar) weight.
     *
     * Identical weights are VERY likely in a project recently
     * converted from the pre-3.0 format, which did not have the
     * test correlation information.
     */
    if (noise_percent > 0)
    {
	if (count_max == 0)
	    count_max = 1;
	frac = count_max * (noise_percent / 100.) / 30000.;
	for (j = 0; j < nitems; ++j)
	    item[j].count += frac * (r250() % 30001);
    }

    /*
     * sort the results by their count
     */
    qsort(item, nitems, sizeof(item[0]), limit_suggestions_cmp);

    /*
     * transfer the results
     */
    for (j = 0; j < nitems; ++j)
    {
	lp = item + j;
	if (j < want)
	{
	    sub_context_ty  *scp;

	    string_list_append(flp, lp->filename);

	    /*
	     * Verbose information message, so the user will
	     * know why the tests were chosen.
	     */
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", lp->filename);
	    sub_var_set_format
	    (
		scp,
		"Number",
		"scp, %6.2f",
		100. * lp->count / item[0].count
	    );
	    change_verbose(cp, scp, i18n("test $filename scored $number"));
	    sub_context_delete(scp);
	}
	str_free(lp->filename);
    }
    mem_free(item);

    /*
     * Sort the filenames, so that the tests will be executed in the
     * same order as by aet -reg.
     */
    string_list_sort(flp);
}


static void
verbose_message(project_ty *pp, change_ty *cp, batch_result_list_ty *brlp)
{
    sub_context_ty  *scp;

    if (brlp->pass_count)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", brlp->pass_count);
	if (cp)
	    change_verbose(cp, scp, i18n("passed $number tests"));
	else
	    project_verbose(pp, scp, i18n("passed $number tests"));
	sub_context_delete(scp);
    }
    if (brlp->no_result_count)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", brlp->no_result_count);
	if (brlp->fail_count)
	{
	    if (cp)
		change_error(cp, scp, i18n("no result $number tests"));
	    else
		project_error(pp, scp, i18n("no result $number tests"));
	}
	else
	{
	    if (cp)
		change_fatal(cp, scp, i18n("no result $number tests"));
	    else
		project_fatal(pp, scp, i18n("no result $number tests"));
	}
	sub_context_delete(scp);
    }
    if (brlp->fail_count)
    {
	scp = sub_context_new();
	sub_var_set_long(scp, "Number", brlp->fail_count);
	if (cp)
	    change_fatal(cp, scp, i18n("failed $number tests"));
	else
	    project_fatal(pp, scp, i18n("failed $number tests"));
	/* NOTREACHED */
	sub_context_delete(scp);
    }
}


static void
test_main(void)
{
    sub_context_ty  *scp;
    int		    baseline_flag;
    int		    regression_flag;
    int		    manual_flag;
    int		    automatic_flag;
    int		    progress_flag;
    string_list_ty  wl;
    string_list_ty  wl2;
    string_ty	    *s1;
    string_ty	    *s2;
    fstate_src_ty   *p_src_data;
    cstate_ty	    *cstate_data;
    fstate_src_ty   *c_src_data =    0;
    string_ty	    *dir; /* unresolved */
    size_t	    j;
    size_t	    k;
    string_ty	    *project_name;
    project_ty	    *pp;
    long	    change_number;
    change_ty	    *cp;
    log_style_ty    log_style;
    user_ty	    *up;
    string_list_ty  search_path;
    time_t	    when;
    int		    integrating;
    int             reviewing;
    int		    suggest;
    int		    suggest_noise;
    int		    based;
    string_ty	    *base;
    int		    exempt;
    int		    force;
    batch_result_list_ty *brlp;

    trace(("test_main()\n{\n"));
    arglex();
    project_name = 0;
    change_number = 0;
    baseline_flag = 0;
    regression_flag = 0;
    manual_flag = 0;
    automatic_flag = 0;
    force = 0;
    log_style = log_style_snuggle_default;
    string_list_constructor(&wl);
    dir = 0;
    suggest = 0;
    suggest_noise = -1;
    progress_flag = -1;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(test_usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    /* fall through... */

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, test_usage);
	    continue;

	case arglex_token_regression:
	    if (regression_flag)
		duplicate_option(test_usage);
	    regression_flag = 1;
	    break;

	case arglex_token_manual:
	    if (manual_flag)
		duplicate_option(test_usage);
	    manual_flag = 1;
	    break;

	case arglex_token_automatic:
	    if (automatic_flag)
		duplicate_option(test_usage);
	    automatic_flag = 1;
	    break;

	case arglex_token_force:
	    if (force)
		duplicate_option(test_usage);
	    force = 1;
	    break;

	case arglex_token_progress:
	    if (progress_flag == 1)
		duplicate_option(test_usage);
	    if (progress_flag >= 0)
	    {
		too_many_progress:
		mutually_exclusive_options
		(
		    arglex_token_progress,
		    arglex_token_progress_not,
		    test_usage
		);
	    }
	    progress_flag = 1;
	    break;

	case arglex_token_progress_not:
	    if (progress_flag == 0)
		duplicate_option(test_usage);
	    if (progress_flag >= 0)
		goto too_many_progress;
	    progress_flag = 0;
	    break;

	case arglex_token_file:
	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		test_usage();
	    /* fall through... */

	case arglex_token_string:
	    s2 = str_from_c(arglex_value.alv_string);
	    string_list_append(&wl, s2);
	    str_free(s2);
	    break;

	case arglex_token_baseline:
	    if (baseline_flag)
		duplicate_option(test_usage);
	    baseline_flag = 1;
	    break;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, test_usage);
	    continue;

	case arglex_token_nolog:
	    if (log_style == log_style_none)
		duplicate_option(test_usage);
	    log_style = log_style_none;
	    break;

	case arglex_token_persevere:
	case arglex_token_persevere_not:
	    user_persevere_argument(test_usage);
	    break;

	case arglex_token_suggest:
	    if (suggest)
		duplicate_option(test_usage);
	    if (arglex() != arglex_token_number)
	    {
		suggest = -1;
		continue;
	    }
	    suggest = arglex_value.alv_number;
	    if (suggest <= 0)
	    {
		scp = sub_context_new();
		sub_var_set_charstar
		(
		    scp,
		    "Name",
		    arglex_token_name(arglex_token_suggest)
		);
		sub_var_set_long(scp, "Number", arglex_value.alv_number);
		error_intl(scp, i18n("$name $number must be pos"));
		sub_context_delete(scp);
		test_usage();
	    }
	    break;

	case arglex_token_suggest_noise:
	    if (suggest_noise >= 0)
		duplicate_option(test_usage);
	    if (arglex() != arglex_token_number)
		option_needs_number(arglex_token_suggest_noise, test_usage);
	    suggest_noise = arglex_value.alv_number;
	    if (suggest_noise < 0 || suggest_noise > 100)
	    {
		scp = sub_context_new();
		sub_var_set_charstar
		(
		    scp,
		    "Name",
		    arglex_token_name(arglex_token_suggest_noise)
		);
		sub_var_set_long(scp, "Number", arglex_value.alv_number);
		error_intl(scp, i18n("$name $number must not be neg"));
		sub_context_delete(scp);
		test_usage();
	    }
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(test_usage);
	    break;
	}
	arglex();
    }
    if (suggest_noise >= 0 && !suggest)
	suggest = -1;
    if (suggest)
	regression_flag = 1;
    if (suggest_noise < 0)
	suggest_noise = 10;
    if (wl.nstrings)
    {
	if (automatic_flag)
	{
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_automatic)
	    );
	    fatal_intl(scp, i18n("no file with $name"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	if (manual_flag)
	{
	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_manual)
	    );
	    fatal_intl(scp, i18n("no file with $name"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	if (regression_flag)
	{
	    scp = sub_context_new();
	    if (suggest)
		sub_var_set_charstar
		(
		    scp,
		    "Name",
		    arglex_token_name(arglex_token_suggest)
		);
	    else
		sub_var_set_charstar
		(
		    scp,
		    "Name",
		    arglex_token_name(arglex_token_regression)
		);
	    fatal_intl(scp, i18n("no file with $name"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
    }
    else
    {
	if (!manual_flag && !automatic_flag)
	{
	    manual_flag = 1;
	    automatic_flag = 1;
	}
    }
    trace_int(manual_flag);
    trace_int(automatic_flag);

    os_throttle();
    when = now();

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
     * take a lock on the change
     */
    change_cstate_lock_prepare(cp);
    project_baseline_read_lock_prepare(pp);
    lock_take();
    cstate_data = change_cstate_get(cp);

    /*
     * see if it is an appropriate thing to be doing
     */
    integrating = 0;
    reviewing = 0;
    switch (cstate_data->state)
    {
    case cstate_state_being_developed:
	if (change_is_a_branch(cp))
	{
	    change_fatal(cp, 0, i18n("bad branch test"));
	}
	if (!str_equal(change_developer_name(cp), user_name(up)))
	    change_fatal(cp, 0, i18n("not developer"));
	if (baseline_flag)
	    dir = project_baseline_path_get(pp, 0);
	else
	{
	    dir = change_development_directory_get(cp, 0);
	    trace_string(dir->str_text);
	}
	break;

    case cstate_state_being_reviewed:
	if
	(
	    project_develop_end_action_get(pp)
	==
	    pattr_develop_end_action_goto_awaiting_review
	)
	{
	    if (!str_equal(change_reviewer_name(cp), user_name(up)))
		change_fatal(cp, 0, i18n("not reviewer"));
	}
	else
	{
	    if (!project_reviewer_query(pp, user_name(up)))
		project_fatal(pp, 0, i18n("not a reviewer"));
	}
	reviewing = 1;
	break;

    case cstate_state_being_integrated:
	if (!str_equal(change_integrator_name(cp), user_name(up)))
	    change_fatal(cp, 0, i18n("not integrator"));
	if (baseline_flag)
	    dir = project_baseline_path_get(pp, 0);
	else
	    dir = change_integration_directory_get(cp, 0);
	integrating = 1;
	force = 1;
	break;

    default:
	change_fatal(cp, 0, i18n("bad test state"));
    }
    assert(dir);

    /*
     * when integrating, must do all the files,
     * may not name any on the command line
     */
    if (integrating && wl.nstrings)
	change_fatal(cp, 0, i18n("int must test all"));

    /*
     * see if this is a complete change test.
     * If it is, we can update the relevant test time field.
     */
    if (regression_flag && !suggest)
	change_regression_test_time_set(cp, when);

    /*
     * Search path for resolving filenames.
     */
    change_search_path_get(cp, &search_path, 1);

    /*
     * Find the base for relative filenames.
     */
    based =
	(
	    search_path.nstrings >= 1
	&&
	    (
		user_relative_filename_preference
		(
		    up,
		    uconf_relative_filename_preference_base
		)
	    ==
		uconf_relative_filename_preference_base
	    )
	);
    if (based)
	base = str_copy(search_path.string[0]);
    else
    {
	os_become_orig();
	base = os_curdir();
	os_become_undo();
    }

    /*
     * check that the named files make sense
     */
    string_list_constructor(&wl2);
    for (j = 0; j < wl.nstrings; ++j)
    {
	string_list_ty	wl_in;

	s1 = wl.string[j];
	if (s1->str_text[0] == '/')
	    s2 = str_copy(s1);
	else
	    s2 = str_format("%S/%S", base, s1);
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
	    sub_var_set_string(scp, "File_Name", wl.string[j]);
	    change_fatal(cp, scp, i18n("$filename unrelated"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}

	/*
	 * see if they named a directory
	 */
	change_file_directory_query(cp, s2, &wl_in, 0);
	if (wl_in.nstrings)
	{
	    int		    used;
	    string_ty	    *s3;

	    used = 0;
	    for (k = 0; k < wl_in.nstrings; ++k)
	    {
		s3 = wl_in.string[k];
		c_src_data = change_file_find(cp, s3);
		assert(c_src_data);
		if (!c_src_data)
		    continue;
		switch (c_src_data->usage)
		{
		case file_usage_test:
		case file_usage_manual_test:
		    if (string_list_member(&wl2, s3))
		    {
			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", s3);
			change_fatal(cp, scp, i18n("too many $filename"));
			/* NOTREACHED */
			sub_context_delete(scp);
		    }
		    else
			string_list_append(&wl2, s3);
		    used = 1;
		    break;

		case file_usage_source:
		case file_usage_config:
		case file_usage_build:
		    break;
		}
	    }
	    if (!used)
	    {
		scp = sub_context_new();
		if (s2->str_length)
		    sub_var_set_string(scp, "File_Name", s2);
		else
		    sub_var_set_charstar(scp, "File_Name", ".");
		sub_var_set_long(scp, "Number", (long)wl_in.nstrings);
		sub_var_optional(scp, "Number");
		project_fatal
		(
		    pp,
		    scp,
		    i18n("directory $filename contains no relevant files")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    str_free(s2);
	    string_list_destructor(&wl_in);
	    continue;
	}
	string_list_destructor(&wl_in);

	/*
	 * make sure the explicitly named file is appropriate
	 */
	c_src_data = change_file_find(cp, s2);
	if (c_src_data)
	{
	    switch (c_src_data->usage)
	    {
	    case file_usage_test:
		break;

	    case file_usage_manual_test:
		log_style = log_style_none;
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
#ifndef DEBUG
	    default:
#endif
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_fatal(cp, scp, i18n("$filename not test"));
		/* NOTREACHED */
		sub_context_delete(scp);
		break;
	    }
	    switch (c_src_data->action)
	    {
	    case file_action_remove:
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_fatal(cp, scp, i18n("$filename being removed"));
		/* NOTREACHED */
		sub_context_delete(scp);
		break;

	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		break;
	    }
	    if (string_list_member(&wl2, s2))
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_fatal(cp, scp, i18n("too many $filename"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    else
		string_list_append(&wl2, s2);
	}
	else
	{
	    p_src_data = project_file_find(pp, s2, view_path_extreme);
	    if (!p_src_data)
	    {
		p_src_data = project_file_find_fuzzy(pp, s2, view_path_extreme);
		if (p_src_data)
		{
		    scp = sub_context_new();
		    sub_var_set_string(scp, "File_Name", s2);
		    sub_var_set_string(scp, "Guess", p_src_data->file_name);
		    project_fatal
		    (
			pp,
			scp,
			i18n("no $filename, closest is $guess")
		    );
		    /* NOTREACHED */
		    sub_context_delete(scp);
		}
		else
		{
		    scp = sub_context_new();
		    sub_var_set_string(scp, "File_Name", s2);
		    change_fatal(cp, scp, i18n("no $filename"));
		    /* NOTREACHED */
		    sub_context_delete(scp);
		}
	    }
	    switch (p_src_data->usage)
	    {
	    case file_usage_test:
		break;

	    case file_usage_manual_test:
		log_style = log_style_none;
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
#ifndef DEBUG
	    default:
#endif
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		project_fatal(pp, scp, i18n("$filename not test"));
		/* NOTREACHED */
		sub_context_delete(scp);
		break;
	    }
	    if (string_list_member(&wl2, s2))
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s2);
		change_fatal(cp, scp, i18n("too many $filename"));
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    else
		string_list_append(&wl2, s2);
	}
	str_free(s2);
    }
    string_list_destructor(&search_path);
    string_list_destructor(&wl);
    wl = wl2;

    if (automatic_flag || manual_flag)
    {
	assert(!wl.nstrings);
	if (regression_flag)
	{
	    /*
	     * run through the project test files
	     */
	    for (j = 0;; ++j)
	    {
		p_src_data = project_file_nth(pp, j, view_path_extreme);
		if (!p_src_data)
		    break;

		/*
		 * don't run the test if it is being
		 * modified by the change
		 */
		if (change_file_find(cp, p_src_data->file_name))
		    continue;

		/*
		 * run the test if it satisfies the request
		 */
		switch (p_src_data->usage)
		{
		case file_usage_test:
		    if (automatic_flag)
			string_list_append(&wl, p_src_data->file_name);
		    break;

		case file_usage_manual_test:
		    if (manual_flag)
			string_list_append(&wl, p_src_data->file_name);
		    break;

		case file_usage_source:
		case file_usage_config:
		case file_usage_build:
		    break;
		}
	    }
	}
	else
	{
	    for (j = 0;; ++j)
	    {
		c_src_data = change_file_nth(cp, j);
		if (!c_src_data)
		    break;
		switch (c_src_data->action)
		{
		case file_action_remove:
		    continue;

		case file_action_create:
		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
		    break;
		}
		switch (c_src_data->usage)
		{
		case file_usage_test:
		    if (automatic_flag)
			string_list_append(&wl, c_src_data->file_name);
		    break;

		case file_usage_manual_test:
		    if (manual_flag)
		    {
			string_list_append(&wl, c_src_data->file_name);
			log_style = log_style_none;
		    }
		    break;

		case file_usage_source:
		case file_usage_config:
		case file_usage_build:
		    break;
		}
	    }
	}
    }

    /*
     * make sure we are actually doing something
     */
    if (suggest)
	exempt = 0;
    else if (wl.nstrings)
	exempt = 0;
    else if (regression_flag)
    {
	exempt = cstate_data->regression_test_exempt;
    }
    else
    {
	if (baseline_flag)
	    exempt = cstate_data->test_baseline_exempt;
	else
	    exempt = cstate_data->test_exempt;
    }
    if (!wl.nstrings)
    {
	if (exempt)
	    quit(0);
	if (regression_flag)
	    project_fatal(pp, 0, i18n("proj has no tests"));
	change_fatal(cp, 0, i18n("has no tests"));
    }

    /*
     * It is an error if the change attributes include architectures
     * not in the project.
     */
    change_check_architectures(cp);

    /*
     * open the log file as the appropriate user
     */
    if (reviewing)
    {
	/* do nothing */
    }
    else if (integrating)
    {
	user_ty		*pup;

	pup = project_user(pp);
	log_open(change_logfile_get(cp), pup, log_style);
	user_free(pup);
    }
    else
	log_open(change_logfile_get(cp), up, log_style);

    /*
     * Limit the suggestions.
     *
     * Do this after the log file is open, so that the weightings
     * are recorded in the log file.
     */
    if (suggest)
	limit_suggestions(cp, &wl, suggest, suggest_noise);
    else if (!force)
    {
	string_list_constructor(&wl2);
	for (j = 0; j < wl.nstrings; ++j)
	{
	    string_ty	    *fn;
	    time_t	    last_time;
	    fstate_src_ty   *src_data;

	    /*
	     * this only applies to change tests,
	     * not project tests
	     */
	    fn = wl.string[j];
	    src_data = change_file_find(cp, fn);
	    if (!src_data)
	    {
		string_list_append(&wl2, fn);
		continue;
	    }

	    /*
	     * verify that out idea of the file's mtime
	     * is up-to-date
	     */
	    if (!integrating)
	    {
		change_file_fingerprint_check(cp, src_data);
		assert(src_data->file_fp);
		assert(src_data->file_fp->oldest>=0);
		assert(src_data->file_fp->youngest>=0);
	    }

	    /*
	     * Check to see if we need to run the test at all.
	     */
	    if (baseline_flag)
		last_time = change_file_test_baseline_time_get(cp, src_data, 0);
	    else
		last_time = change_file_test_time_get(cp, src_data, 0);
	    assert(src_data->file_fp);
	    assert(src_data->file_fp->youngest>=0);
	    assert(src_data->file_fp->oldest>=0);
	    if (!last_time || src_data->file_fp->oldest >= last_time)
		string_list_append(&wl2, fn);
	}
	string_list_destructor(&wl);
	wl = wl2;
    }

    /*
     * Do each of the tests.
     */
    trace_string(dir->str_text);
    if (progress_flag < 0)
	progress_flag = 0;
    brlp = change_test_run_list(cp, &wl, up, baseline_flag, progress_flag);

    /*
     * transcribe the results
     */
    trace(("brlp->length = %d\n", (int)brlp->length));
    for (j = 0; j < brlp->length; ++j)
    {
	batch_result_ty *rp;
	fstate_src_ty   *src_data;

	rp = &brlp->item[j];
	trace(("rp->filename = \"%s\"\n", rp->file_name->str_text));
	trace(("rp->exit_status = %d\n", rp->exit_status));
	src_data = change_file_find(cp, rp->file_name);
	if (src_data)
	{
	    switch (rp->exit_status)
	    {
	    case 0:
		if (baseline_flag)
		{
		    change_file_test_baseline_time_clear
		    (
			cp,
			src_data,
			rp->architecture
		    );
		}
		else
		{
		    change_file_test_time_set
		    (
			cp,
			src_data,
			when,
			rp->architecture
		    );
		}
		break;

	    case 1:
		if (baseline_flag)
		{
		    change_file_test_baseline_time_set
		    (
			cp,
			src_data,
			when,
			rp->architecture
		    );
		}
		else
		{
		    change_file_test_time_clear(cp, src_data, rp->architecture);
		}
		break;

	    default:
		if (baseline_flag)
		{
		    change_file_test_baseline_time_clear
		    (
			cp,
			src_data,
			rp->architecture
		    );
		}
		else
		{
		    change_file_test_time_clear(cp, src_data, rp->architecture);
		}
		break;
	    }
	}
	else
	{
	    if (rp->exit_status)
		change_regression_test_time_set(cp, (time_t)0);
	}
    }

    /*
     * write out the data
     */
    if (!reviewing)
    {
	change_cstate_write(cp);
	commit();
    }
    lock_release();

    /*
     * verbose result message
     */
    verbose_message(pp, cp, brlp);
    if (brlp->no_result_count || brlp->fail_count)
	quit(1);

    /*
     * clean up and go home
     */
    batch_result_list_delete(brlp);
    change_free(cp);
    project_free(pp);
    user_free(up);
    trace(("}\n"));
}


static void
test_independent(void)
{
    int		    automatic_flag;
    int		    manual_flag;
    int		    progress_flag;
    string_list_ty  wl;
    string_list_ty  wl2;
    string_list_ty  wl_in;
    string_ty	    *s1;
    string_ty	    *s2;
    string_ty	    *s3;
    fstate_src_ty   *src_data;
    size_t	    j;
    size_t	    k;
    string_ty	    *project_name;
    project_ty	    *pp;
    user_ty	    *pup;
    user_ty	    *up;
    string_list_ty  search_path;
    int		    based;
    string_ty	    *base;
    batch_result_list_ty *brlp;

    trace(("test_independent()\n{\n"));
    project_name = 0;
    automatic_flag = 0;
    manual_flag = 0;
    progress_flag = -1;
    string_list_constructor(&wl);
    arglex();
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(test_usage);
	    continue;

	case arglex_token_manual:
	    if (manual_flag)
		duplicate_option(test_usage);
	    manual_flag = 1;
	    break;

	case arglex_token_automatic:
	    if (automatic_flag)
		duplicate_option(test_usage);
	    automatic_flag = 1;
	    break;

	case arglex_token_progress:
	    if (progress_flag == 1)
		duplicate_option(test_usage);
	    if (progress_flag >= 0)
	    {
		too_many_progress:
		mutually_exclusive_options
		(
		    arglex_token_progress,
		    arglex_token_progress_not,
		    test_usage
		);
	    }
	    progress_flag = 1;
	    break;

	case arglex_token_progress_not:
	    if (progress_flag == 0)
		duplicate_option(test_usage);
	    if (progress_flag >= 0)
		goto too_many_progress;
	    progress_flag = 0;
	    break;

	case arglex_token_directory:
	    if (arglex() != arglex_token_string)
		option_needs_dir(arglex_token_directory, test_usage);
	    goto get_file_names;

	case arglex_token_file:
	    if (arglex() != arglex_token_string)
		option_needs_files(arglex_token_file, test_usage);
	    /* fall through... */

	case arglex_token_string:
	    get_file_names:
	    s2 = str_from_c(arglex_value.alv_string);
	    string_list_append(&wl, s2);
	    str_free(s2);
	    break;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, test_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_lock_wait_argument(test_usage);
	    break;

	case arglex_token_base_relative:
	case arglex_token_current_relative:
	    user_relative_filename_preference_argument(test_usage);
	    break;

	case arglex_token_persevere:
	case arglex_token_persevere_not:
	    user_persevere_argument(test_usage);
	    break;
	}
	arglex();
    }
    if (wl.nstrings)
    {
	if (automatic_flag)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_automatic)
	    );
	    error_intl(scp, i18n("no file with $name"));
	    sub_context_delete(scp);
	    test_usage();
	}
	if (manual_flag)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_charstar
	    (
		scp,
		"Name",
		arglex_token_name(arglex_token_manual)
	    );
	    error_intl(scp, i18n("no file with $name"));
	    sub_context_delete(scp);
	    test_usage();
	}
    }
    else
    {
	if (!automatic_flag && !manual_flag)
	{
	    automatic_flag = 1;
	    manual_flag = 1;
	}
    }

    /*
     * locate project data
     */
    if (!project_name)
	project_name = user_default_project();
    pp = project_alloc(project_name);
    str_free(project_name);
    project_bind_existing(pp);
    pup = project_user(pp);

    /*
     * locate user data
     */
    up = user_executing(pp);

    /*
     * Take a baseline read lock.
     */
    project_baseline_read_lock_prepare(pp);
    lock_take();

    /*
     * Search path for resolving filenames.
     */
    string_list_constructor(&search_path);
    project_search_path_get(pp, &search_path, 1);

    /*
     * Find the base for relative filenames.
     */
    based =
	(
	    search_path.nstrings >= 1
	&&
	    (
		user_relative_filename_preference
		(
		    up,
		    uconf_relative_filename_preference_base
		)
	    ==
		uconf_relative_filename_preference_base
	    )
	);
    if (based)
	base = str_copy(search_path.string[0]);
    else
    {
	os_become_orig();
	base = os_curdir();
	os_become_undo();
    }

    /*
     * make sure the paths make sense
     */
    string_list_constructor(&wl2);
    for (j = 0; j < wl.nstrings; ++j)
    {
	s1 = wl.string[j];
	if (s1->str_text[0] == '/')
	    s2 = str_copy(s1);
	else
	    s2 = str_format("%S/%S", base, s1);
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
	    sub_var_set_string(scp, "File_Name", wl.string[j]);
	    project_fatal(pp, scp, i18n("$filename unrelated"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}

	/*
	 * check to see if a directory was named
	 */
	project_file_directory_query(pp, s2, &wl_in, 0, view_path_extreme);
	if (wl_in.nstrings)
	{
	    int		    used;

	    used = 0;
	    for (k = 0; k < wl_in.nstrings; ++k)
	    {
		s3 = wl_in.string[k];
		src_data = project_file_find(pp, s3, view_path_extreme);
		if (!src_data)
		    continue;
		switch (src_data->usage)
		{
		case file_usage_test:
		case file_usage_manual_test:
		    if (string_list_member(&wl2, s3))
		    {
			sub_context_ty	*scp;

			scp = sub_context_new();
			sub_var_set_string(scp, "File_Name", s3);
			project_fatal(pp, scp, i18n("too many $filename"));
			/* NOTREACHED */
			sub_context_delete(scp);
		    }
		    else
			string_list_append(&wl2, s3);
		    used = 1;
		    break;

		case file_usage_source:
		case file_usage_config:
		case file_usage_build:
		    break;
		}
	    }
	    if (!used)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		if (s2->str_length)
		    sub_var_set_string(scp, "File_Name", s2);
		else
		    sub_var_set_charstar(scp, "File_Name", ".");
		sub_var_set_long(scp, "Number", (long)wl_in.nstrings);
		sub_var_optional(scp, "Number");
		project_fatal
		(
		    pp,
		    scp,
		    i18n("directory $filename contains no relevant files")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	    }
	    str_free(s2);
	    string_list_destructor(&wl_in);
	    continue;
	}
	string_list_destructor(&wl_in);

	/*
	 * make sure the explicitly named file is relevant
	 */
	src_data = project_file_find(pp, s2, view_path_extreme);
	if (!src_data)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s2);
	    project_fatal(pp, scp, i18n("no $filename"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	switch (src_data->usage)
	{
	    sub_context_ty  *scp;

	case file_usage_test:
	case file_usage_manual_test:
	    break;

	case file_usage_source:
	case file_usage_config:
	case file_usage_build:
#ifndef DEBUG
	default:
#endif
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s2);
	    project_fatal(pp, scp, i18n("$filename not test"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	    break;
	}
	if (string_list_member(&wl2, s2))
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", s2);
	    project_fatal(pp, scp, i18n("too many $filename"));
	    /* NOTREACHED */
	    sub_context_delete(scp);
	}
	else
	    string_list_append(&wl2, s2);
    }
    string_list_destructor(&wl);
    wl = wl2;
    string_list_destructor(&search_path);

    /*
     * snarf the test names from the project
     */
    if (automatic_flag || manual_flag)
    {
	for (j = 0;; ++j)
	{
	    src_data = project_file_nth(pp, j, view_path_extreme);
	    if (!src_data)
		break;
	    switch (src_data->usage)
	    {
	    case file_usage_test:
		if (automatic_flag)
		    string_list_append(&wl, src_data->file_name);
		break;

	    case file_usage_manual_test:
		if (manual_flag)
		    string_list_append(&wl, src_data->file_name);
		break;

	    case file_usage_source:
	    case file_usage_config:
	    case file_usage_build:
		break;
	    }
	}
	if (!wl.nstrings)
	    project_fatal(pp, 0, i18n("has no tests"));
    }

    /*
     * do each of the tests
     * (Logging is disabled, because there is no [logical] place
     * to put the log file; the user should redirect stdout and stderr.)
     */
    if (progress_flag < 0)
	progress_flag = 0;
    brlp = project_test_run_list(pp, &wl, up, progress_flag);

    /*
     * Release the baseline read lock.
     */
    lock_release();

    /*
     * verbose result message
     */
    verbose_message(pp, (change_ty *)0, brlp);

    /*
     * clean up and go home
     */
    batch_result_list_delete(brlp);
    user_free(up);
    project_free(pp);
    trace(("}\n"));
}


void
test(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{arglex_token_independent, test_independent, },
	{arglex_token_help, test_help, },
	{arglex_token_list, test_list, },
    };

    trace(("test()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), test_main);
    trace(("}\n"));
}
