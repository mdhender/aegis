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
// MANIFEST: functions to manipulate project_bogus
//

#include <file_info.h>
#include <input.h>
#include <module/private.h>
#include <module/project_bogu.h>
#include <server.h>


struct module_project_bogus_ty
{
    module_ty       inherited;
    string_ty       *name;
};


static void
destructor(module_ty *mp)
{
    module_project_bogus_ty *mpbp;

    mpbp = (module_project_bogus_ty *)mp;
    str_free(mpbp->name);
    mpbp->name = 0;
}


static void
groan(module_ty *mp, server_ty *sp, const char *caption)
{
    module_project_bogus_ty *mpbp;

    mpbp = (module_project_bogus_ty *)mp;
    server_error
    (
	sp,
	"%s: project \"%s\": no such project",
	caption,
	mpbp->name->str_text
    );
}


static void
modified(module_ty *mp, server_ty *sp, string_ty *file_name, file_info_ty *fip,
    input_ty *ip)
{
    groan(mp, sp, "Modified");
}


static string_ty *
canonical_name(module_ty *mp)
{
    // FIXME: memory leak
    return str_from_c("no such project");
}


static int
update(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt)
{
    groan(mp, sp, "update");
    return 0;
}


static int
checkin(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side)
{
    groan(mp, sp, "ci");
    return 0;
}


static int
add(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt)
{
    groan(mp, sp, "add");
    return 0;
}


static int
remove(module_ty *mp, server_ty *sp, string_ty *client_side,
    string_ty *server_side, module_options_ty *opt)
{
    groan(mp, sp, "remove");
    return 0;
}


static const module_method_ty vtbl =
{
    sizeof(module_project_bogus_ty),
    destructor,
    modified,
    canonical_name,
    update,
    checkin,
    add,
    remove,
    1, // bogus
    "project bogus"
};


module_ty *
module_project_bogus_new(string_ty *name)
{
    module_ty       *mp;
    module_project_bogus_ty *mpbp;

    mp = module_new(&vtbl);
    mpbp = (module_project_bogus_ty *)mp;
    mpbp->name = str_copy(name);
    return mp;
}
