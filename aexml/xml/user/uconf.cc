//
//	aegis - project change supervisor
//	Copyright (C) 2003-2005 Peter Miller;
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
// MANIFEST: functions to manipulate uconfs
//

#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/project/inappropriat.h>
#include <libaegis/output.h>
#include <common/trace.h>
#include <libaegis/user.h>
#include <aexml/xml/user/uconf.h>


void
xml_user_uconf(string_ty *project_name, long change_number, output_ty *op)
{
    user_ty         *up;

    trace(("xml_user_uconf()\n{\n"));
    if (project_name)
	list_project_inappropriate();
    if (change_number)
	list_change_inappropriate();
    up = user_executing(0);
    user_uconf_write_xml(up, op);
    trace(("}\n"));
}
