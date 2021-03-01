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
// MANIFEST: functions to manipulate finds
//

#include <ac/stdlib.h>
#include <ac/string.h>

#include <module/change.h>
#include <module/cvsroot.h>
#include <module/project.h>
#include <symtab.h>


static int
extract_change_number(string_ty **project_name_p, long *change_number_p)
{
    string_ty       *project_name;
    const char      *cp;
    char            *end;
    long            change_number;

    project_name = *project_name_p;
    cp = strstr(project_name->str_text, ".C");
    if (!cp)
	cp = strstr(project_name->str_text, ".c");
    if (!cp)
	return 0;
    change_number = strtol(cp + 2, &end, 10);
    if (end == cp + 2 || *end)
	return 0;

    //
    // We have a change number.
    //
    // NOTE: the caller must free the new project name.
    //
    *change_number_p = change_number;
    *project_name_p =
	str_n_from_c(project_name->str_text, cp - project_name->str_text);
    return 1;
}


static void
reaper(void *p)
{
    module_delete((module_ty *)p);
}


module_ty *
module_find(string_ty *name)
{
    static symtab_ty *stp;
    module_ty       *mp;

    //
    // We cache the answers.  A typical session is going to do this
    // for many requests, not just one.
    //
    if (!stp)
    {
	stp = symtab_alloc(5);
	stp->reap = reaper;
    }
    mp = (module_ty *)symtab_query(stp, name);
    if (mp)
    {
	mp->reference_count++;
	return mp;
    }

    //
    // We haven't seen this one before,
    // create a new instance for it.
    //
    if (0 == strcmp(name->str_text, "CVSROOT"))
        mp = module_cvsroot_new();
    else
    {
	long            change_number;

	if (extract_change_number(&name, &change_number))
	{
	    mp = module_change_new(name, change_number);
	    str_free(name);
	}
	else
	    mp = module_project_new(name);
    }

    //
    // Stash it away in the symbol table.
    //
    // We bump the reference count so that the symbol table always has
    // a valid reference.  Maybe the symbol table belongs to the server
    // (or net), so it can be cleaned up when the server goes away.
    //
    mp->reference_count++;
    symtab_assign(stp, name, mp);

    //
    // Report success.
    //
    return mp;
}


module_ty *
module_find_trim(string_ty *arg)
{
    string_ty       *name;
    module_ty       *mp;

    name = str_field(arg, '/', 0);
    mp = module_find(name);
    str_free(name);
    return mp;
}
