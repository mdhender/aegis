//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
project_ty::home_path_set(const nstring &s)
{
    home_path_set(s.get_ref());
}


void
project_ty::home_path_set(string_ty *s)
{
    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    trace(("project_ty::home_path_set(this = %08lX, s = \"%s\")\n{\n",
	(long)this, s->str_text));
    if (parent)
	this_is_a_bug();
    if (home_path)
    {
	sub_context_ty sc;
	sc.var_set_charstar("Name", arglex_token_name(arglex_token_directory));
	sc.fatal_intl(i18n("duplicate $name option"));
	// NOTREACHED
    }
    home_path = str_copy(s);

    //
    // set it in the trunk change, too
    //
    change_development_directory_set(change_get(), s);
    trace(("}\n"));
}
