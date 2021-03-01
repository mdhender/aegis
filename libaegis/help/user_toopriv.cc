//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2003-2007 Peter Miller
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
// MANIFEST: functions to manipulate user_tooprivs
//

#include <common/nstring.h>
#include <libaegis/help.h>
#include <libaegis/sub.h>


void
fatal_user_too_privileged(string_ty *name)
{
    fatal_user_too_privileged(nstring(name));
}


void
fatal_user_too_privileged(const nstring &name)
{
    sub_context_ty sc;
    sc.var_set_string("Name", name);
    sc.fatal_intl(i18n("user $name too privileged"));
}
