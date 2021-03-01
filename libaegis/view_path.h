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
 * MANIFEST: interface definition for libaegis/view_path.c
 */

#ifndef LIBAEGIS_VIEW_PATH_H
#define LIBAEGIS_VIEW_PATH_H

#include <main.h>

/*
 * If you add to this enum, make sure you extend project_ty::file_list
 * and change_ty::file_list
 */
enum view_path_ty
{
    /*
     * Do not impose a view path when searching for files.
     */
    view_path_none,

    /*
     * Apply the transparency when searching for files, but return
     * removed files when you see them.
     */
    view_path_simple,

    /*
     * Apply the transparency when searching for files, and also omit
     * any mention of removed files.
     */
    view_path_extreme
};
typedef enum view_path_ty view_path_ty;

/**
  * The view_path_ename function may be used to obtain a human readable
  * string equivalent of a view_path_ty value.
  */
const char *view_path_ename(view_path_ty);

#endif /* LIBAEGIS_VIEW_PATH_H */
