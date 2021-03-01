//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the project_ty::home_path_get method
//

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/gonzo.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


string_ty *
project_ty::home_path_get()
{
    trace(("project_ty::home_path_get(this = %08lX))\n{\n", (long)this));
    if (parent)
	this_is_a_bug();
    if (!home_path)
    {
	string_ty	*s;

	//
	// it is an error if the project name is not known
	//
	s = gonzo_project_home_path_from_name(name);
	if (!s)
	{
	    sub_context_ty sc;
	    sc.var_set_string("Name", name);
	    sc.fatal_intl(i18n("no $name project"));
	    // NOTREACHED
	}

	//
	// To cope with automounters, directories are stored as given,
	// or are derived from the home directory in the passwd file.
	// Within aegis, pathnames have their symbolic links resolved,
	// and any comparison of paths is done on this "system idea"
	// of the pathname.
	//
	home_path = str_copy(s);
    }
    trace(("return \"%s\";\n", home_path->str_text));
    trace(("}\n"));
    return home_path;
}
