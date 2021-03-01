/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: interface definition for libaegis/project/file/list_get.c
 */

#ifndef LIBAEGIS_PROJECT_FILE_LIST_GET_H
#define LIBAEGIS_PROJECT_FILE_LIST_GET_H

#include <project/file.h>

/**
  * The project_file_list_get function is used to obtain a list of file names.
  *
  * \param pp
  *	The project to be scanned.
  * \param as_view_path
  *	If false, list all files.  If true, omit files which have been removed.
  */
struct string_list_ty *project_file_list_get(struct project_ty *pp,
    view_path_ty as_view_path);

#endif /* LIBAEGIS_PROJECT_FILE_LIST_GET_H */
