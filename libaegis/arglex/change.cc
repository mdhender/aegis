//
//      aegis - project change supervisor
//      Copyright (C) 2003-2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/ctype.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/mem.h>
#include <common/uuidentifier.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/change.h>
#include <libaegis/help.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>


static inline char
safe_toupper(char c)
{
    if (islower((unsigned char)c))
        return toupper((unsigned char)c);
    return c;
}


static inline bool
safe_isdigit(char c)
{
    return isdigit((unsigned char)c);
}


static inline bool
safe_ispunct(char c)
{
    return ispunct((unsigned char)c);
}


static bool
extract_change_number(string_ty **project_name_p, long *change_number_p)
{
    string_ty *project_name = *project_name_p;
    if (safe_toupper(project_name->str_text[0]) == 'C')
    {
        const char *cp = project_name->str_text + 1;
        char *end = 0;
        long change_number = strtol(cp, &end, 10);
        if (end == cp || *end)
            return false;
        *change_number_p = change_number;
        *project_name_p = str_from_c("");
    }
    else
    {
        const char *cp = strstr(project_name->str_text, ".C");
        if (!cp)
            cp = strstr(project_name->str_text, ".c");
        if (!cp)
            return false;
        char *end = 0;
        long change_number = strtol(cp + 2, &end, 10);
        if (end == cp + 2 || *end)
            return false;
        *change_number_p = change_number;
        string_ty *new_project_name =
            str_n_from_c(project_name->str_text, cp - project_name->str_text);
        *project_name_p = new_project_name;
    }
    str_free(project_name);
    return true;
}


static bool
extract_delta_number(string_ty **project_name_p, long *delta_number_p)
{
    string_ty *project_name = *project_name_p;
    if (safe_toupper(project_name->str_text[0]) == 'D')
    {
        const char *cp = project_name->str_text + 1;
        char *end = 0;
        long delta_number = strtol(cp, &end, 10);
        if (end == cp || *end)
            return false;
        if (delta_number <= 0)
            return false;
        *delta_number_p = delta_number;
        *project_name_p = str_from_c("");
    }
    else
    {
        const char *cp = strstr(project_name->str_text, ".D");
        if (!cp)
            cp = strstr(project_name->str_text, ".d");
        if (!cp)
            return false;
        char *end = 0;
        long delta_number = strtol(cp + 2, &end, 10);
        if (end == cp + 2 || *end)
            return false;
        if (delta_number <= 0)
            return false;
        *delta_number_p = delta_number;
        *project_name_p =
            str_n_from_c(project_name->str_text, cp - project_name->str_text);
    }
    str_free(project_name);
    return true;
}


static bool
is_a_branch_number(string_ty *s)
{
    if (!s->str_length)
        return true;
    bool digit_required = true;
    const char *cp = s->str_text;
    for (;;)
    {
        if (digit_required)
        {
            if (!*cp)
                return false;
            if (!safe_isdigit(*cp))
                return false;
            digit_required = false;
        }
        else
        {
            if (!*cp)
                return true;
            if (safe_isdigit(*cp))
                digit_required = false;
            else if (safe_ispunct(*cp))
                digit_required = true;
            else
                return false;
        }
        ++cp;
    }
}


static void
no_such_uuid(project *pp, string_ty *uuid)
{
    //
    // Build the message we actually want to send.
    //
    sub_context_ty *scp = sub_context_new();
    string_ty *msg = subst_intl(scp, i18n("unknown change"));

    //
    // Get ready to pass the message to the project error function.
    // We are going to re-use the substitution context.
    //
    sub_var_set_string(scp, "MeSsaGe", msg);
    str_free(msg);

    //
    // Wrap the project name and chnage "number" around the error message.
    //
    sub_var_set_string(scp, "Change", uuid);
    sub_var_override(scp, "Change");
    project_fatal(pp, scp, i18n("change $change: $message"));
    // NOTREACHED
}


/**
  * The arglex_parse_change_tok is used to parse a complete --change
  * command line option and any succeeding arguments.  When finished,
  * the parse point will be placed at the begining of the next command
  * line option.
  */
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
            // NOTREACHED
        }
        break;

    case arglex_token_string:
        s = str_from_c(arglex_value.alv_string);
        if (universal_unique_identifier_valid_partial(s))
        {
            //
            // Hunt for the change set with the given UUID within the
            // project.  Complain if there is no change set with the
            // given UUID if the UUID is exact.  Move on if there is no
            // match, or too many matches, for a partial (prefix) UUID.
            //
            if (!project_name)
            {
                nstring n = user_ty::create()->default_project();
                project_name = n.get_ref_copy();
            }
            project *pp = project_alloc(project_name);
            pp->bind_existing();
            change::pointer cp = project_uuid_find(pp, s);
            if (!cp)
            {
                if (s->str_length == 36)
                {
                    no_such_uuid(pp, s);
                    // NOTREACHED
                }
                project_free(pp);
            }
            else
            {
                if (!*project_name_p)
                    *project_name_p = project_name_get(cp->pp).get_ref_copy();
                else if (nstring(project_name) != project_name_get(cp->pp))
                    duplicate_option_by_name(arglex_token_project, usage);
                change_number = cp->number;
                if (change_number == 0)
                    change_number = MAGIC_ZERO;
                change_free(cp);
                project_free(pp);
                project_name = *project_name_p;
                break;
            }
        }
        if (extract_change_number(&s, &change_number))
        {
            if (is_a_branch_number(s) && !project_name)
            {
                project      *pp;
                project      *pp2;

                nstring n = user_ty::create()->default_project();
                project_name = n.get_ref_copy();
                pp = project_alloc(project_name);
                pp->bind_existing();
                pp2 = pp->find_branch(s->str_text);
                project_name = project_name_get(pp2).get_ref_copy();
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
                project      *pp;
                project      *pp2;

                nstring n = user_ty::create()->default_project();
                project_name = n.get_ref_copy();
                pp = project_alloc(project_name);
                pp->bind_existing();
                pp2 = pp->find_branch(s->str_text);
                project_name = project_name_get(pp2).get_ref_copy();
                change_number =
                    project_delta_number_to_change_number(pp2, delta_number);
                project_free(pp2);
                project_free(pp);
                str_free(s);
            }
            else
            {
                project      *pp;

                if (project_name && !str_equal(project_name, s))
                    duplicate_option_by_name(arglex_token_project, usage);
                project_name = s;

                pp = project_alloc(project_name);
                pp->bind_existing();
                change_number =
                    project_delta_number_to_change_number(pp, delta_number);
                project_free(pp);
            }
            break;
        }
        // fall through...

    default:
        option_needs_number(token, usage);
        // NOTREACHED
    }

    // advance past change number
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
            // NOTREACHED
        }
        break;

    case arglex_token_string:
        s = str_from_c(arglex_value.alv_string);
        if (universal_unique_identifier_valid_partial(s))
        {
            //
            // Hunt for the change set with the given UUID within the
            // project.  Complain if there is no change set with the
            // given UUID if the UUID is exact.  Move on if there is no
            // match, or too many matches, for a partial (prefix) UUID.
            //
            if (!project_name)
            {
                nstring n = user_ty::create()->default_project();
                project_name = n.get_ref_copy();
            }
            project *pp = project_alloc(project_name);
            pp->bind_existing();
            change::pointer cp = project_uuid_find(pp, s);
            if (!cp)
            {
                if (s->str_length == 36)
                {
                    no_such_uuid(pp, s);
                    // NOTREACHED
                }
                project_free(pp);
                pp = 0;
            }
            else
            {
                if (!*project_name_p)
                    *project_name_p = project_name_get(cp->pp).get_ref_copy();
                else if (nstring(project_name) != project_name_get(cp->pp))
                    duplicate_option_by_name(arglex_token_project, usage);
                change_number = cp->number;
                if (change_number == 0)
                    change_number = MAGIC_ZERO;
                change_free(cp);
                cp = 0;
                project_free(pp);
                pp = 0;
                project_name = *project_name_p;
                break;
            }
        }
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
                project      *pp;
                project      *pp2;

                if (branch && 0 != strcmp(branch, s->str_text))
                    duplicate_option_by_name(arglex_token_branch, usage);
                branch = mem_copy_string(s->str_text);
                str_free(s);

                //
                // Bind to the project.
                //
                if (!project_name)
                {
                    nstring n = user_ty::create()->default_project();
                    project_name = n.get_ref_copy();
                }
                pp = project_alloc(project_name);
                pp->bind_existing();
                pp2 = pp->find_branch(branch);

                //
                // Find the change number.
                //
                change_number =
                    project_delta_number_to_change_number(pp2, delta_number);
                project_free(pp2);
                project_free(pp);
            }
            else
            {
                project      *pp;

                if (project_name && !str_equal(project_name, s))
                    duplicate_option_by_name(arglex_token_project, usage);
                project_name = s;

                pp = project_alloc(project_name);
                pp->bind_existing();
                change_number =
                    project_delta_number_to_change_number(pp, delta_number);
                project_free(pp);
            }
            break;
        }
        // fall through...

    default:
        option_needs_number(arglex_token_change, usage);
        // NOTREACHED
    }

    // advance past change number
    arglex();

    *branch_p = branch;
    *change_number_p = change_number;
    *project_name_p = project_name;
}


// vim: set ts=8 sw=4 et :
