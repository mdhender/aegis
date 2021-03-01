//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h> // for assert

#include <aecvsserver/module/change.h>
#include <aecvsserver/module/cvsroot.h>
#include <aecvsserver/module/project.h>
#include <common/symtab.h>


static bool
extract_change_number(string_ty **project_name_p, long *change_number_p)
{
    string_ty       *project_name;
    char            *end;
    long            change_number;

    project_name = *project_name_p;
    const char *cp =
        (const char *)
        memmem(project_name->str_text, project_name->str_length, ".C", 2);
    if (!cp)
    {
	cp =
            (const char *)
            memmem(project_name->str_text, project_name->str_length, ".c", 2);
    }
    if (!cp)
	return false;
    change_number = strtol(cp + 2, &end, 10);
    if (end == cp + 2 || *end)
	return false;

    //
    // We have a change number.
    //
    // NOTE: the caller must free the new project name.
    //
    *change_number_p = change_number;
    *project_name_p =
	str_n_from_c(project_name->str_text, cp - project_name->str_text);
    return true;
}


static void
reaper(void *p)
{
    module_ty *mp = (module_ty *)(p);
    mp->reference_count_down();
}


module
module::find(string_ty *name)
{
    //
    // We cache the answers.  A typical session is going to do this
    // for many requests, not just one.
    //
    static symtab_ty *stp;
    if (!stp)
    {
	stp = new symtab_ty(5);
	stp->set_reap(reaper);
    }
    module_ty *mp = (module_ty *)stp->query(name);
    if (mp)
    {
        //
        // We increase the reference count, so that the symbol table
        // always has a valid reference.
	//
	assert(mp->reference_count_valid());
	mp->reference_count_up();
	return mp;
    }

    //
    // We haven't seen this one before,
    // create a new instance for it.
    //
    if (0 == strcmp(name->str_text, "CVSROOT"))
        mp = new module_cvsroot();
    else
    {
	long change_number = 0;
	if (extract_change_number(&name, &change_number))
	    mp = module_change_new(name, change_number);
	else
	    mp = module_project_new(name);
    }

    //
    // Stash it away in the symbol table.
    //
    // The reference count is one when allocated.  The symbol table
    // has a valid reference at this point.
    //
    stp->assign(name, mp);
    string_ty *name2 = mp->name();
    if (!str_equal(name, name2))
    {
	//
        // This isn't supposed to happen, because the client calls
        // expand-modules forst.
	//
	mp->reference_count_up();
	stp->assign(name2, mp);
    }

    //
    // We increase the reference count, so that the symbol table
    // always has a valid reference AND the caller has a valid reference.
    //
    mp->reference_count_up();
    assert(mp->reference_count_valid());

    //
    // Report success.
    //
    return mp;
}


module
module::find_trim(string_ty *arg)
{
    string_ty *name = str_field(arg, '/', 0);
    module m = find(name);
    str_free(name);
    return m;
}
