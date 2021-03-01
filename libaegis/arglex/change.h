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

#ifndef LIBAEGIS_ARGLEX_CHANGE_H
#define LIBAEGIS_ARGLEX_CHANGE_H

#include <common/str.h>

/**
  * The arglex_parse_change function is used to parse a change number,
  * once the -Change command line option has been seen.  It is expected
  * that arglex() has been called, so that the currect command line
  * token is the one following the -Change token.
  *
  * Does not return on error.  On a successful return, the current token
  * has been advanced past the change numnber.
  */
void arglex_parse_change(string_ty **project_name_p, long *change_number_p,
    void (*usage)(void));

/**
  * The arglex_parse_change function is used to parse a brancg number,
  * once the -BRanch command line option has been seen.  It is expected
  * that arglex() has been called, so that the currect command line
  * token is the one following the -BRanch token.
  *
  * Does not return on error.  On a successful return, the current token
  * has been advanced past the branch numnber.
  */
void arglex_parse_branch(string_ty **project_name_p, long *change_number_p,
    void (*usage)(void));

/**
  * The arglex_parse_change_with_branch function is used to parse a
  * change number, once the -Change command line option has been seen.
  * It is expected that arglex() has been called, so that the current
  * command line token is the one following the -Change token.
  *
  * The simplest form is a change number.  The next form allows 1.2.C345
  * so the branch may be specified along with the change number.
  *
  * Does not return on error.  On a successful return, the current token
  * has been advanced past the change numnber.
  */
void arglex_parse_change_with_branch(string_ty **project_name_p,
    long *change_number, const char **branch_p, void (*usage)(void));

#endif // LIBAEGIS_ARGLEX_CHANGE_H
