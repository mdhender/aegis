//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
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
// MANIFEST: functions to manipulate projects
//

#include <arglex2.h>
#include <arglex/project.h>
#include <help.h>


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
