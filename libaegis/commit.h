/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1993, 2002, 2004 Peter Miller.
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
 * MANIFEST: interface definition for aegis/commit.c
 */

#ifndef COMMIT_H
#define COMMIT_H

#include <str.h>

/**
  * The commit_rename function is used to
  * submit a commit request to rename a file.
  *
  * \param from
  *     path of file now
  * \param to
  *     path of file after commit
  */
void commit_rename(string_ty *from, string_ty *to);

/**
  * The commit_symlink function is used to submit a commit request to
  * symlink a file at commit (success) time.
  *
  * \param from
  *     path of file now
  * \param to
  *     path of symlink to create at commit time.
  */
void commit_symlink(string_ty *from, string_ty *to);

/**
  * The commit_unlink_errok function is used to unlink a file on
  * commit.  It will not be an error if the file does not exist.
  *
  * \param path
  *     path of file to be unlinked
  */
void commit_unlink_errok(string_ty *path);

/**
  * The commit_rmdir_errok function is used to delete an empty
  * directory on commit.  It will not be an error if the directory
  * does not exist.  It will not be an error if the directory is not
  * empty.
  *
  * \param path
  *     path of directory to be deleted
  */
void commit_rmdir_errok(string_ty *path);

/**
  * The commit_rmdir_tree_bg function is used to delete a directory tree
  * on commit.  It will not be an error if the directory does not exist.
  * It will not be an error if the directory, or any subtree, is not
  * empty.
  *
  * \param path
  *     path of directory to be deleted
  */
void commit_rmdir_tree_bg(string_ty *path);

/**
  * The commit_rmdir_tree_errok function is used to delete a directory
  * tree on commit.  It will not be an error if the directory does not
  * exist.  It will not be an error if the directory, or any subtree,
  * is not empty.
  *
  * \param path
  *     path of directory to be deleted
  */
void commit_rmdir_tree_errok(string_ty *path);

/**
  * The commit function is used to perform all the actions queued using
  * the commit_* functions.
  *
  * After it has completed successfully, further calls to commit()
  * will be NOPs, until new commit_* functions are used.
  *
  * When the commit has succeeded, the undo list is cancelled, since
  * there is now no reason to undo anything.
  */
void commit(void);

#endif /* COMMIT_H */
