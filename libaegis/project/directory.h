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
 * MANIFEST: interface definition for libaegis/project/directory.c
 */

#ifndef LIBAEGIS_PROJECT_DIRECTORY_H
#define LIBAEGIS_PROJECT_DIRECTORY_H

#include <main.h>

struct project_ty; /* forward */

/**
  * The project_directory_get function is used to get a directory for use
  * with the current command.  The project baseline will be returned,
  * unless the branch has been closed, in which case the branch tree
  * will be ascended (possibly all the way to the trunk) looking for a
  * branch baseline which is still being developed.
  */
struct string_ty *project_directory_get(struct project_ty *pp, int resolve);

#endif /* LIBAEGIS_PROJECT_DIRECTORY_H */