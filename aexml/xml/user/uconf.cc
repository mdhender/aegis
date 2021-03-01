//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/output.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <aexml/xml/user/uconf.h>


void
xml_user_uconf(string_ty *project_name, long change_number, output::pointer op)
{
    user_ty::pointer up;

    trace(("xml_user_uconf()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();
    up = user_ty::create();
    up->uconf_write_xml(op);
    trace(("}\n"));
}
