//
//	aegis - project change supervisor
//	Copyright (C) 2002, 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_DIR_STACK_H
#define LIBAEGIS_DIR_STACK_H

#include <common/main.h>
#include <common/str_list.h>

enum dir_stack_walk_message_t
{
	dir_stack_walk_dir_before,
	dir_stack_walk_dir_after,
	dir_stack_walk_file,
	dir_stack_walk_special,
	dir_stack_walk_symlink
};

struct stat; // forward

/**
  * The TOP_LEVEL_SYMLINK flags is returned in the depth_p field to
  * indicate that there is a top-level symbolic link obscuring the file.
  * This is only used when the "ignore_symlinks" flags is used.
  */
#define TOP_LEVEL_SYMLINK (~(~0u >> 1))

/**
  * The dir_stack_readdir function is used to read the path directory
  * for its member file names, relative to the stack provided.  Members
  * present in more than one directory will only be mentioned once.
  */
void dir_stack_readdir(string_list_ty *stack, string_ty *path,
    string_list_ty *result);

/**
  * The dir_stack_walk_callback_t type is the type for the callback of
  * the dir_stack_walk function.
  *									<p>
  * The `arg' argument is passed through unchanged by the dir_walk_stack
  * function.  It may be used to pass auxilliary data without he use of
  * global variables.
  *									<p>
  * The `statbuf' argument is the system information about the file
  * or directory.
  *									<p>
  * The `msg' argument is used to inform the callback as to what type of
  * file has occurred.  While this information may also be determoined
  * from statbuf, it also informs you when you are having a pre or post
  * look at a directory.  When used in a switch, gcc will warn you if you
  * accidentally omit one of the enum values (unlike the S_ISxxx macros).
  *									<p>
  * The `relpath' argument is the name of the file or directory relative
  * to the base of the viewpath.
  *									<p>
  * The `ignore_symlinks' argument is used to indicate whether
  * dir_stack_walk was called with the same argument.
  *									<p>
  * The `depth' argument is how deep in the view path the file occurred.
  * In addition, if ignore_symlinks was specified, and there was a
  * top-level symbolic link obscuring the file, the TOP_LEVEL_SYMLINK
  * bit will be set.
  */
typedef void (*dir_stack_walk_callback_t)(void *arg,
    dir_stack_walk_message_t msg, string_ty *relpath, struct stat *statbuf,
    int depth, int ignore_symlinks);

/**
  * The dir_stack_walk function is used to recursively walk a directory
  * tree.  However, unlike the dir_walk function, the directory is the
  * logical union of all the directories specified in the `viewpath'.
  *									<p>
  * The `viewpath' argument is used to specify the list of directories
  * to be unioned together.  They shall all be absolute paths.
  *									<p>
  * The `path' argument is used to specify, relative to the base of the
  * viewpath, the file or directory to walk.
  *									<p>
  * The `callback' argument is used to specify the function to be called
  * for each file or directory found in the viewpath.
  *									<p>
  * The `arg' argument is used to so that you may transmit auxilliary
  * data to the callback without global variables.  It is passwd through
  * unchanged.
  *									<p>
  * The 'ignore_symlinks' argument is used to specify that symlinks should
  * be ignored in the view path when looking for files.  Only if there
  * are no files will the callback be called with information about a
  * symbolic link, otherwise it will be called with information about
  * the shallowest non-symlink.
  */
void dir_stack_walk(string_list_ty *viewpath, string_ty *path,
    dir_stack_walk_callback_t callback, void *arg, int ignore_symlinks);

/**
  * The dir_stack_find function is used to locate a file in the viewpath.
  * The return value is the absolute path of the file, with the view
  * path resolved.
  *									<p>
  * The `viewpath' argument is used to specify the list of directories
  * to be unioned together.  They shall all be absolute paths.
  *									<p>
  * The `filename' argument is used to specify, relative to the base of
  * the viewpath, the file or directory to walk.
  *
  * The `start_pos' argument is used to indicate where in the view path
  * to start looking.
  *									<p>
  * The 'ignore_symlinks' argument is used to specify that symlinks
  * should be ignored in the view path when looking for files.  Only if
  * there are no files will the function return information about a
  * symbolic link, otherwise it will be called with information about
  * the shallowest non-symlink.
  *									<p>
  * The 'statbuf' argument is used to return the system, information
  * about the file or directory.
  *									<p>
  * The `depth_p' argument is used to return information about how deep
  * in the view path the file occurred.  In addition, if ignore_symlinks
  * was specified, and there was a top-level symbolic link obscuring
  * the file, the TOP_LEVEL_SYMLINK bit will be set.
  */
string_ty *dir_stack_find(string_list_ty *viewpath, size_t start_pos,
	string_ty *path, struct stat *statbuf, int *depth,
	int ignore_symlinks);

/**
  * The dir_stack_stat function is almost identical to the dir_stat_find
  * function.  It searches the entire viewpath for the given filename.
  * It generates a fatal error if the file is not found, and does
  * not return.
  *									<p>
  * The `viewpath' argument is used to specify the list of directories
  * to be unioned together.  They shall all be absolute paths.
  *									<p>
  * The `filename' argument is used to specify, relative to the base of
  * the viewpath, the file or directory to walk.
  *
  * The 'ignore_symlinks' argument is used to specify that symlinks
  * should be ignored in the view path when looking for files.  Only if
  * there are no files will the function return information about a
  * symbolic link, otherwise it will be called with information about
  * the shallowest non-symlink.
  *									<p>
  * The 'statbuf' argument is used to return the system, information
  * about the file or directory.
  *									<p>
  * The `depth_p' argument is used to return information about how deep
  * in the view path the file occurred.  In addition, if ignore_symlinks
  * was specified, and there was a top-level symbolic link obscuring
  * the file, the TOP_LEVEL_SYMLINK bit will be set.
  */
void dir_stack_stat(string_list_ty *viewpath, string_ty *filename,
	struct stat *statbuf, int *depth, int ignore_symlinks);

/**
  * The dir_stack_relative function is used to scan a view path and return
  * the relative name of the given absolute path.  It returns the null
  * pointer of the absolute path is not below any of the directories in
  * the view path.
  *									<p>
  * The `viewpath' argument is used to specify the list of directories
  * to be unioned together.  They shall all be absolute paths.
  *
  * The `abspath' argument is the absolute path to be unresolved.
  */
string_ty *dir_stack_relative(string_list_ty *viewpath, string_ty *abspath);

#endif // LIBAEGIS_DIR_STACK_H
