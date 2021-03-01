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

#ifndef LIBAEGIS_VIEW_PATH_H
#define LIBAEGIS_VIEW_PATH_H

#include <common/main.h>

/**
  * If you add to this enum, make sure you extend project_ty::file_list
  * and change::file_list
  */
enum view_path_ty
{
    /**
      * Only use the first element of the view path.  Transparent and
      * removed files will be reported.
      */
    view_path_first,

    /**
      * Walk down the search list looking for files, unioning all of
      * the file lists together.  Transparent and removed file will be
      * reported.  The "none" is somewhat of a misnomer - it means no
      * transparency processing is done, rather then no view path.
      */
    view_path_none,

    /**
      * Apply the transparency when searching for files, but return
      * removed files when you see them.
      */
    view_path_simple,

    /**
      * Apply the transparency when searching for files, and also omit
      * any mention of removed files.
      */
    view_path_extreme
};

//
// Use for declaring arrays indexed by view_path_ty.
// Don't put it into the enum, of gcc can't give sensable errors.
//
#define view_path_MAX 4

/**
  * The view_path_ename function may be used to obtain a human readable
  * string equivalent of a view_path_ty value.
  */
const char *view_path_ename(view_path_ty);

#endif // LIBAEGIS_VIEW_PATH_H
