//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_ARGLEX_PROJECT_H
#define LIBAEGIS_ARGLEX_PROJECT_H

#include <common/str.h>

/**
  * The arglex_parse_project function is used to parse a project name,
  * once the -Project command line option has been seen.  It is expected
  * that arglex() has been called, so that the current command line
  * token is the one following the -Project token.
  *
  * Does not return on error.  On a successful return, the current token
  * has been advanced past the project name.
  */
void arglex_parse_project(string_ty **project_name_p, void (*usage)(void));

#endif // LIBAEGIS_ARGLEX_PROJECT_H
