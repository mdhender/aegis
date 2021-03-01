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

#include <aeget/get/command.h>
#include <aeget/get/project/aedist.h>
#include <common/libdir.h>
#include <libaegis/project.h>
#include <common/str_list.h>


void
get_project_aedist(project_ty *pp, string_ty *, string_list_ty *)
{
    string_ty       *qp;
    string_ty       *command;

    qp = str_quote_shell(project_name_get(pp));
    command =
	str_format
	(
	    "%s/aedist -send -bl -p %s -cte=none -mime-header -compress -ndh",
	    configured_bindir(),
	    qp->str_text
	);
    str_free(qp);
    get_command(command->str_text);
    // NOTREACHED
}
