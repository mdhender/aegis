//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006 Peter Miller;
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
// MANIFEST: functions to manipulate change_boguss
//

#include <aecvsserver/file_info.h>
#include <libaegis/input.h>
#include <aecvsserver/module/change_bogus.h>
#include <aecvsserver/server.h>


module_change_bogus::~module_change_bogus()
{
    str_free(pname);
    pname = 0;
    number = 0;
}


module_change_bogus::module_change_bogus(string_ty *arg1, long arg2) :
    pname(str_copy(arg1)),
    number(arg2)
{
}


void
module_change_bogus::groan(server_ty *sp, const char *caption)
{
    server_error
    (
	sp,
	"%s: project \"%s\": change %ld: no such change",
	caption,
	pname->str_text,
	number
    );
}


void
module_change_bogus::modified(server_ty *sp, string_ty *file_name,
    file_info_ty *fip, input &ip)
{
    groan(sp, "Modified");
}


bool
module_change_bogus::update(server_ty *sp, string_ty *client_side,
    string_ty *server_side, const options &opt)
{
    groan(sp, "update");
    return false;
}


bool
module_change_bogus::checkin(server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    groan(sp, "ci");
    return false;
}


bool
module_change_bogus::add(server_ty *sp, string_ty *client_side,
    string_ty *server_side, const options &opt)
{
    groan(sp, "add");
    return false;
}


bool
module_change_bogus::remove(server_ty *sp, string_ty *client_side,
    string_ty *server_side, const options &opt)
{
    groan(sp, "remove");
    return false;
}


string_ty *
module_change_bogus::calculate_canonical_name()
    const
{
    // FIXME: memory leak
    return str_from_c("no such change");
}


bool
module_change_bogus::is_bogus()
    const
{
    return true;
}
