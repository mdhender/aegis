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
// MANIFEST: functions to manipulate existings
//

#include <ac/ctype.h>
#include <ac/stdlib.h>

#include <change/branch.h>
#include <error.h> // for assert
#include <fstrcmp.h>
#include <gonzo.h>
#include <project.h>
#include <str_list.h>
#include <sub.h>
#include <trace.h>
#include <zero.h>


static int
name_has_numeric_suffix(string_ty *name, string_ty **left, long *right)
{
    char	    *ep;

    ep = name->str_text + name->str_length;
    while (ep > name->str_text && isdigit((unsigned char)ep[-1]))
	--ep;
    if (!*ep)
    // still pointing to end of string
	return 0;
    if (ep < name->str_text + 2 || !ispunct((unsigned char)ep[-1]))
	return 0;
    *left = str_n_from_c(name->str_text, ep - 1 - name->str_text);
    *right = magic_zero_encode(atol(ep));
    return 1;
}


void
project_bind_existing(project_ty *pp)
{
    string_ty	    *s;
    string_ty	    *parent_name;
    int		    alias_retry_count;

    //
    // make sure project exists
    //
    trace(("project_bind_existing(pp = %08lX)\n{\n", (long)pp));
    assert(!pp->home_path);
    alias_retry_count = 0;
    alias_retry:
    s = gonzo_project_home_path_from_name(pp->name);

    //
    // If the named project was not found, and it has a numeric suffix,
    // then assume that it is a project.branch combination,
    // and try to find the deeper project.
    //
    if (!s && name_has_numeric_suffix(pp->name, &parent_name, &pp->parent_bn))
    {
	pp->parent = project_alloc(parent_name);
	project_bind_existing(pp->parent);
	pp->pcp = change_alloc(pp->parent, pp->parent_bn);
	change_bind_existing(pp->pcp);
	if (!change_was_a_branch(pp->pcp))
	    change_fatal(pp->pcp, 0, i18n("not a branch"));

	//
	// rebuild the project name
	//	...eventually, use the remembered punctuation
	//
	str_free(pp->name);
	pp->name =
	    str_format
	    (
		"%s.%ld",
		project_name_get(pp->parent)->str_text,
		magic_zero_decode(pp->parent_bn)
	    );

	pp->changes_path =
	    str_format
	    (
		"%s.branch",
		project_change_path_get(pp->parent, pp->parent_bn)->str_text
	    );
	return;
    }

    //
    // If the name was not found, try to find an alias match for it.
    // Loop if an alias is found.
    //
    if (!s)
    {
	string_ty	*other;

	other = gonzo_alias_to_actual(pp->name);
	if (other)
	{
	    if (++alias_retry_count > 5)
	    {
		project_fatal(pp, 0, i18n("alias loop detected"));
	    }
	    str_free(pp->name);
	    pp->name = str_copy(other);
	    goto alias_retry;
	}
    }

    //
    // If the name was not found, try to find a fuzzy match for it.
    // In general, this results in more informative error messages.
    //
    if (!s)
    {
	string_list_ty	wl;
	string_ty	*best;
	double		best_weight;
	size_t          j;

	gonzo_project_list(&wl);
	best = 0;
	best_weight = 0.6;
	for (j = 0; j < wl.nstrings; ++j)
	{
	    double	    w;

	    s = wl.string[j];
	    w = fstrcmp(pp->name->str_text, s->str_text);
	    if (w > best_weight)
	    {
		best = s;
		best_weight = w;
	    }
	}
	if (best)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", pp->name);
	    sub_var_set_string(scp, "Guess", best);
	    fatal_intl(scp, i18n("no $name project, closest is $guess"));
	}
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", pp->name);
	    fatal_intl(scp, i18n("no $name project"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    pp->home_path = str_copy(s);
    trace(("}\n"));
}


int
project_bind_existing_errok(project_ty *pp)
{
    string_ty	    *s;
    string_ty	    *parent_name;
    int		    alias_retry_count;

    //
    // make sure project exists
    //
    trace(("project_bind_existing_errok(pp = %08lX)\n{\n", (long)pp));
    assert(!pp->home_path);
    alias_retry_count = 0;
    alias_retry:
    s = gonzo_project_home_path_from_name(pp->name);

    //
    // If the named project was not found, and it has a numeric suffix,
    // then assume that it is a project.branch combination,
    // and try to find the deeper project.
    //
    if (!s && name_has_numeric_suffix(pp->name, &parent_name, &pp->parent_bn))
    {
	pp->parent = project_alloc(parent_name);
	if (!project_bind_existing_errok(pp->parent))
	{
	    trace(("return 0;\n"));
	    trace(("}\n"));
	    return 0;
	}
	pp->pcp = change_alloc(pp->parent, pp->parent_bn);
	if (!change_bind_existing_errok(pp->pcp))
	{
	    trace(("return 0;\n"));
	    trace(("}\n"));
	    return 0;
	}
	if (!change_was_a_branch(pp->pcp))
	{
	    trace(("return 0;\n"));
	    trace(("}\n"));
	    return 0;
	}

	//
	// rebuild the project name
	//	...eventually, use the remembered punctuation
	//
	str_free(pp->name);
	pp->name =
	    str_format
	    (
		"%s.%ld",
		project_name_get(pp->parent)->str_text,
		magic_zero_decode(pp->parent_bn)
	    );

	pp->changes_path =
	    str_format
	    (
		"%s.branch",
		project_change_path_get(pp->parent, pp->parent_bn)->str_text
	    );
	trace(("return 1;\n"));
	trace(("}\n"));
	return 1;
    }

    //
    // If the name was not found, try to find an alias match for it.
    // Loop if an alias is found.
    //
    if (!s)
    {
	string_ty	*other;

	other = gonzo_alias_to_actual(pp->name);
	if (other)
	{
	    if (++alias_retry_count > 5)
	    {
		trace(("return 0;\n"));
		trace(("}\n"));
		return 0;
	    }
	    str_free(pp->name);
	    pp->name = str_copy(other);
	    goto alias_retry;
	}
    }

    //
    // If the name was not found, try to find a fuzzy match for it.
    // In general, this results in more informative error messages.
    //
    if (!s)
    {
	trace(("return 0;\n"));
	trace(("}\n"));
	return 0;
    }

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    pp->home_path = str_copy(s);
    trace(("return 1\n"));
    trace(("}\n"));
    return 1;
}
