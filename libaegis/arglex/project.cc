//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#include <libaegis/arglex2.h>
#include <libaegis/arglex/project.h>
#include <libaegis/help.h>


void
arglex_parse_project(string_ty **project_name_p, void (*usage)(void))
{
    if (arglex_token != arglex_token_string)
	option_needs_name(arglex_token_project, usage);
    if (*project_name_p)
	duplicate_option_by_name(arglex_token_project, usage);
    *project_name_p = str_from_c(arglex_value.alv_string);
    arglex();
}
