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

#include <aecvsserver/file_info.h>
#include <libaegis/input.h>
#include <aecvsserver/module/project_bogu.h>
#include <aecvsserver/server.h>


module_project_bogus::~module_project_bogus()
{
    str_free(projname);
    projname = 0;
}


module_project_bogus::module_project_bogus(string_ty *arg) :
    projname(str_copy(arg))
{
}


void
module_project_bogus::groan(server_ty *sp, const char *caption)
{
    server_error
    (
	sp,
	"%s: project \"%s\": no such project",
	caption,
	projname->str_text
    );
}


void
module_project_bogus::modified(server_ty *sp, string_ty *, file_info_ty *,
    input &)
{
    groan(sp, "Modified");
}


string_ty *
module_project_bogus::calculate_canonical_name()
    const
{
    // FIXME: memory leak
    return str_from_c("no such project");
}


bool
module_project_bogus::update(server_ty *sp, string_ty *, string_ty *,
    const options &)
{
    groan(sp, "update");
    return false;
}


bool
module_project_bogus::checkin(server_ty *sp, string_ty *, string_ty *)
{
    groan(sp, "ci");
    return false;
}


bool
module_project_bogus::add(server_ty *sp, string_ty *, string_ty *,
    const options &)
{
    groan(sp, "add");
    return false;
}


bool
module_project_bogus::remove(server_ty *sp, string_ty *, string_ty *,
    const options &)
{
    groan(sp, "remove");
    return 0;
}


bool
module_project_bogus::is_bogus()
    const
{
    return true;
}
