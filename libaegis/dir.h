//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 2002, 2004-2006, 2008 Peter Miller.
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

#ifndef LIBAEGIS_DIR_H
#define LIBAEGIS_DIR_H

#include <common/nstring.h>

class dir_functor; // forward

enum dir_walk_message_ty
{
    dir_walk_dir_before,
    dir_walk_dir_after,
    dir_walk_file,
    dir_walk_special,
    dir_walk_symlink
};

struct stat; // forward

typedef void (*dir_walk_callback_ty)(void *arg, dir_walk_message_ty,
	string_ty *, const struct stat *);

/**
  * The dir_walk function is used to recursively walk a directory tree,
  * calling the \a cb funtion for each entry.
  *
  * @param path
  *     The path to the directory to be walked.
  * @param cb
  *     The function to be called for each file and directory.
  * @param arg
  *     A extra argument ot be passed to the \a cb function, usually
  *     used to provide context.
  */
void dir_walk(string_ty *path, dir_walk_callback_ty cb, void *arg);

/**
  * The dir_walk function is used to recursively walk a directory tree,
  * calling the \a cb functor for each entry.
  *
  * @param path
  *     The path to the directory to be walked.
  * @param cb
  *     The functor to be called for each file and directory.
  */
void dir_walk(const nstring &path, dir_functor &cb);

#endif // LIBAEGIS_DIR_H
