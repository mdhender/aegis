/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate changes
 */

#include <ac/ctype.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <arglex2.h>
#include <arglex/change.h>
#include <help.h>
#include <mem.h>
#include <project/history.h>
#include <sub.h>
#include <user.h>
#include <zero.h>


static int
extract_change_number(string_ty **project_name_p, long *change_number_p)
{
    string_ty       *project_name;
    const char      *cp;
    char            *end;
    long            change_number;
    string_ty       *new_project_name;

    project_name = *project_name_p;
    cp = strstr(project_name->str_text, ".C");
    if (!cp)
	cp = strstr(project_name->str_text, ".c");
    if (!cp)
	return 0;
    change_number = strtol(cp + 2, &end, 10);
    if (end == cp + 2 || *end)
	return 0;
    *change_number_p = change_number;
    new_project_name =
	str_n_from_c(project_name->str_text, cp - project_name->str_text);
    str_free(project_name);
    *project_name_p = new_project_name;
    return 1;
}


static int
extract_delta_number(string_ty **project_name_p, long *delta_number_p)
{
    string_ty       *project_name;
    const char      *cp;
    char            *end;
    long            delta_number;
    string_ty       *new_project_name;

    project_name = *project_name_p;
    cp = strstr(project_name->str_text, ".D");
    if (!cp)
	cp = strstr(project_name->str_text, ".d");
    if (!cp)
	return 0;
    delta_number = strtol(cp + 2, &end, 10);
    if (end == cp + 2 || *end)
	return 0;
    if (delta_number <= 0)
	return 0;
    *delta_number_p = delta_number;
    new_project_name =
	str_n_from_c(project_name->str_text, cp - project_name->str_text);
    str_free(project_name);
    *project_name_p = new_project_name;
    return 1;
}


static int
is_a_branch_number(string_ty *s)
{
    const char      *cp;
    int             digit_required;

    if (!s->str_length)
	return 1;
    digit_required = 1;
    cp = s->str_text;
    for (;;)
    {
	if (digit_required)
	{
	    if (!*cp)
		return 0;
	    if (!isdigit((unsigned char)*cp))
		return 0;
	    digit_required = 0;
	}
	else
	{
	    if (!*cp)
		return 1;
	    if (isdigit((unsigned char)*cp))
		digit_required = 0;
	    else if (ispunct((unsigned char)*cp))
		digit_required = 1;
	    else
		return 0;
	}
	++cp;
    }
}


static void
arglex_parse_change_tok(string_ty **project_name_p, long *change_number_p,
    int token, void (*usage)(void))
{
    string_ty       *project_name;
    long            change_number;
    string_ty       *s;
    long            delta_number;

    change_number = *change_number_p;
    project_name = *project_name_p;
    if (change_number)
	duplicate_option_by_name(token, usage);
    switch (arglex_token)
    {
    case arglex_token_number:
	change_number = arglex_value.alv_number;
	range_check:
	if (change_number == 0)
	    change_number = MAGIC_ZERO;
	else if (change_number < 1)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", change_number);
	    if (token == arglex_token_branch)
	       	fatal_intl(scp, i18n("branch $number out of range"));
	    else
		fatal_intl(scp, i18n("change $number out of range"));
	    /* NOTREACHED */
	}
	break;

    case arglex_token_string:
	s = str_from_c(arglex_value.alv_string);
	if (extract_change_number(&s, &change_number))
	{
	    if (is_a_branch_number(s) && !project_name)
	    {
		project_ty      *pp;
		project_ty      *pp2;

		project_name = user_default_project();
		pp = project_alloc(project_name);
		project_bind_existing(pp);
		pp2 = project_find_branch(pp, s->str_text);
		project_name = str_copy(project_name_get(pp2));
		project_free(pp2);
		project_free(pp);
		str_free(s);
	    }
	    else
	    {
		if (project_name && !str_equal(project_name, s))
		    duplicate_option_by_name(arglex_token_project, usage);
		project_name = s;
	    }
	    goto range_check;
	}
	if (extract_delta_number(&s, &delta_number))
	{
	    if (is_a_branch_number(s) && !project_name)
	    {
		project_ty      *pp;
		project_ty      *pp2;

		project_name = user_default_project();
		pp = project_alloc(project_name);
		project_bind_existing(pp);
		pp2 = project_find_branch(pp, s->str_text);
		project_name = str_copy(project_name_get(pp2));
		change_number =
		    project_delta_number_to_change_number(pp2, delta_number);
		project_free(pp2);
		project_free(pp);
		str_free(s);
	    }
	    else
	    {
		project_ty      *pp;

		if (project_name && !str_equal(project_name, s))
		    duplicate_option_by_name(arglex_token_project, usage);
		project_name = s;

		pp = project_alloc(project_name);
		project_bind_existing(pp);
		change_number =
		    project_delta_number_to_change_number(pp, delta_number);
		project_free(pp);
	    }
	    break;
	}
	/* fall through... */

    default:
	option_needs_number(token, usage);
	/* NOTREACHED */
    }
    arglex();
    *change_number_p = change_number;
    *project_name_p = project_name;
}


void
arglex_parse_change(string_ty **project_name_p, long *change_number_p,
    void (*usage)(void))
{
    arglex_parse_change_tok
    (
	project_name_p,
	change_number_p,
	arglex_token_change,
	usage
    );
}


void
arglex_parse_branch(string_ty **project_name_p, long *change_number_p,
    void (*usage)(void))
{
    arglex_parse_change_tok
    (
	project_name_p,
	change_number_p,
	arglex_token_branch,
	usage
    );
}


void
arglex_parse_change_with_branch(string_ty **project_name_p,
    long *change_number_p, const char **branch_p, void (*usage)(void))
{
    string_ty       *project_name;
    long            change_number;
    const char      *branch;
    string_ty       *s;
    long            delta_number;

    project_name = *project_name_p;
    change_number = *change_number_p;
    branch = *branch_p;
    switch (arglex_token)
    {
    case arglex_token_number:
	if (change_number)
	    duplicate_option_by_name(arglex_token_change, usage);
	change_number = arglex_value.alv_number;
	range_check:
	if (change_number == 0)
	    change_number = MAGIC_ZERO;
	else if (change_number < 1)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", change_number);
	    fatal_intl(scp, i18n("change $number out of range"));
	    /* NOTREACHED */
	}
	arglex();
	break;

    case arglex_token_string:
	s = str_from_c(arglex_value.alv_string);
	if (extract_change_number(&s, &change_number))
	{
	    if (is_a_branch_number(s))
	    {
		if (branch && 0 != strcmp(branch, s->str_text))
		    duplicate_option_by_name(arglex_token_branch, usage);
		branch = mem_copy_string(s->str_text);
		str_free(s);
	    }
	    else
	    {
		if (project_name && !str_equal(project_name, s))
		    duplicate_option_by_name(arglex_token_project, usage);
		project_name = s;
	    }
	    goto range_check;
	}
	if (extract_delta_number(&s, &delta_number))
	{
	    if (is_a_branch_number(s))
	    {
		project_ty      *pp;
		project_ty      *pp2;

		if (branch && 0 != strcmp(branch, s->str_text))
		    duplicate_option_by_name(arglex_token_branch, usage);
		branch = mem_copy_string(s->str_text);
		str_free(s);

		/*
		 * Bind to the project.
		 */
		if (!project_name)
		    project_name = user_default_project();
		pp = project_alloc(project_name);
		project_bind_existing(pp);
		pp2 = project_find_branch(pp, branch);

		/*
		 * Find the change number.
		 */
		change_number =
		    project_delta_number_to_change_number(pp2, delta_number);
		project_free(pp2);
		project_free(pp);
	    }
	    else
	    {
		project_ty      *pp;

		if (project_name && !str_equal(project_name, s))
		    duplicate_option_by_name(arglex_token_project, usage);
		project_name = s;

		pp = project_alloc(project_name);
		project_bind_existing(pp);
		change_number =
		    project_delta_number_to_change_number(pp, delta_number);
		project_free(pp);
	    }
	    break;
	}
	/* fall through... */

    default:
	option_needs_number(arglex_token_change, usage);
	/* NOTREACHED */
    }
    *branch_p = branch;
    *change_number_p = change_number;
    *project_name_p = project_name;
}
