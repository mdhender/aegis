//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
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

#include <common/trace.h>
#include <libaegis/getgr_cache.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


void
user_ty::set_gid(int gid, bool weak)
{
    trace(("user_ty::set_gid(this = %08lX, gid = %d)\n{\n", (long)this, gid));
    if (group_id != gid)
    {
        group_id = gid;
        struct group *gr = getgrgid_cached(group_id);
        if (gr)
            group_name = gr->gr_name;
        else if (weak)
            group_name = nstring::format("gid %d", group_id);
        else
        {
            sub_context_ty sc;
            sc.var_set_long("Number", group_id);
            sc.fatal_intl(i18n("gid $number unknown"));
            // NOTREACHED
        }
    }
    trace(("}\n"));
}
