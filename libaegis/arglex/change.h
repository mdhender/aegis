/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for libaegis/arglex/change.c
 */

#ifndef LIBAEGIS_ARGLEX_CHANGE_H
#define LIBAEGIS_ARGLEX_CHANGE_H

#include <str.h>

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
  * The arglex_parse_branch function is almost identical to  the
  * arglex_parse_change() function, except that the error messages talk
  * about branches.
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

#endif /* LIBAEGIS_ARGLEX_CHANGE_H */
