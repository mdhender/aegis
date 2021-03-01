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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/file.h>
#include <libaegis/common.h>
#include <common/env.h>
#include <common/error.h>
#include <libaegis/help.h>
#include <common/language.h>
#include <libaegis/os.h>
#include <common/progname.h>
#include <libaegis/project.h>
#include <common/quit.h>
#include <libaegis/user.h>


enum
{
    arglex_token_view_path = ARGLEX2_MAX,
    arglex_token_usage,
    arglex_token_action
};

static arglex_table_ty argtab[] =
{
    { "-Action", arglex_token_action },
    { "-Usage", arglex_token_usage },
    { "-View_Path", arglex_token_view_path },
    ARGLEX_END_MARKER
};


static void
usage(void)
{
    const char      *prog;

    prog = progname_get();
    fprintf(stderr, "usage: %s [ -p <name> ][ -c <number> ][ <type> ]\n", prog);
    quit(1);
}


static view_path_ty
view_path_find(const char *s)
{
    int             j;

    for (j = 0; j < view_path_MAX; ++j)
    {
	const char      *s2;

	s2 = view_path_ename((view_path_ty)j);
	if (0 == strcasecmp(s, s2))
	    return (view_path_ty)j;
	s2 += 10;
	if (0 == strcasecmp(s, s2))
	    return (view_path_ty)j;
    }
    fatal_raw("view path type \"%s\" unknown", s);
    // NOTREACHED
    return view_path_simple;
}


static int
file_action_find(const char *s)
{
    int             j;

    for (j = 0; j < file_action_max; ++j)
    {
	const char      *s2;

	s2 = file_action_ename((file_action_ty)j);
	if (0 == strcasecmp(s, s2))
	    return j;
	s2 += 12;
	if (0 == strcasecmp(s, s2))
	    return j;
    }
    fatal_raw("file action \"%s\" unknown", s);
    // NOTREACHED
    return file_action_modify;
}


static int
file_usage_find(const char *s)
{
    int             j;

    for (j = 0; j < file_usage_max; ++j)
    {
	const char      *s2;

	s2 = file_usage_ename((file_usage_ty)j);
	if (0 == strcasecmp(s, s2))
	    return j;
	s2 += 11;
	if (0 == strcasecmp(s, s2))
	    return j;
    }
    fatal_raw("file usage \"%s\" unknown", s);
    // NOTREACHED
    return file_usage_source;
}


int
main(int argc, char **argv)
{
    long	    change_number;
    string_ty	    *project_name;
    project_ty	    *pp;
    change::pointer cp;
    user_ty::pointer up;
    size_t	    j;
    view_path_ty    vp;
    int             usage_mask;
    int             action_mask;
    int             usage_column;
    int             action_column;
    int             n;

    os_become_init_mortal();
    arglex2_init3(argc, argv, argtab);
    env_initialize();
    language_init();

    arglex();

    project_name = 0;
    change_number = 0;
    vp = view_path_extreme;
    usage_mask = 0;
    usage_column = 1;
    action_mask = 0;
    action_column = 1;
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(usage);
	    continue;

	case arglex_token_change:
	    arglex();
	    // fall through...

	case arglex_token_number:
	    arglex_parse_change(&project_name, &change_number, usage);
	    continue;

        case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, usage);
	    continue;

        case arglex_token_view_path:
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_view_path, usage);
	    vp = view_path_find(arglex_value.alv_string);
	    break;

        case arglex_token_usage:
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_usage, usage);
	    n = file_usage_find(arglex_value.alv_string);
	    usage_mask |= 1 << n;
	    usage_column = 0;
	    break;

        case arglex_token_action:
	    if (arglex() != arglex_token_string)
		option_needs_string(arglex_token_action, usage);
	    n = file_action_find(arglex_value.alv_string);
	    action_mask |= 1 << n;
	    action_column = 0;
	    break;
	}
	arglex();
    }
    if (usage_mask == 0)
	usage_mask = ~0;
    if (action_mask == 0)
	action_mask = ~0;

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring pn = user_ty::create()->default_project();
	project_name = str_copy(pn.get_ref());
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

    for (j = 0; ; ++j)
    {
	fstate_src_ty   *src;

	src = change_file_nth(cp, j, vp);
	if (!src)
	    break;
	if (!(usage_mask & (1 << src->usage)))
	    continue;
	if (!(action_mask & (1 << src->action)))
	    continue;
	if (usage_column)
	    printf("%s ", file_usage_ename(src->usage));
	if (action_column)
	    printf("%s ", file_action_ename(src->action));
	printf("%s\n", src->file_name->str_text);
    }

    //
    // report success
    //
    quit(0);
    return 0;
}
