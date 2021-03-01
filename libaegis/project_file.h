/*
 *	aegis - project change supervisor
 *	Copyright (C) 1995, 1996, 1997 Peter Miller;
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
 * MANIFEST: interface definition for aegis/project_file.c
 */

#ifndef AEGIS_PROJECT_FILE_H
#define AEGIS_PROJECT_FILE_H

#include <project.h>
#include <fstate.h>

fstate_src project_file_find _((project_ty *, string_ty *));
string_ty *project_file_path _((project_ty *, string_ty *));
fstate_src project_file_find_fuzzy _((project_ty *, string_ty *));
void project_file_dir _((project_ty *, string_ty *, struct string_list_ty *,
	struct string_list_ty *));
fstate_src project_file_new _((project_ty *, string_ty *));
void project_file_remove _((project_ty *, string_ty *));
fstate_src project_file_nth _((project_ty *, size_t));
void project_search_path_get _((project_ty *, struct string_list_ty *, int));
void project_file_shallow _((project_ty *, string_ty *, long));
int project_file_shallow_check _((project_ty *, string_ty *));

#endif /* AEGIS_PROJECT_FILE_H */
