//
//	aegis - project change supervisor
//	Copyright (C) 1991-1994, 2002, 2004-2006, 2008 Peter Miller.
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

#ifndef UNDO_H
#define UNDO_H

#include <libaegis/quit/action/undo.h>
#include <common/str.h>

/** \addtogroup Transaction
  * \ingroup AegisLibrary
  * @{
  */
class nstring; // forward

/**
  * The undo_rename funtion is used to submit an undo request
  * (rollback) to rename a file.
  *
  * \param from
  *      path of file now
  * \param to
  *      path of file after rollback.
  */
void undo_rename(string_ty *from, string_ty *to);

/**
  * The undo_rename funtion is used to submit an undo request
  * (rollback) to rename a file.
  *
  * \param from
  *      path of file now
  * \param to
  *      path of file after rollback.
  */
void undo_rename(const nstring &from, const nstring &to);

/**
  * The undo_rename_cancel function is used to cancel an undo request
  * submitted with undo_rename().
  *
  * \param from
  *     path of file now
  * \param to
  *     path of file_after rollback.
  */
void undo_rename_cancel(string_ty *from, string_ty *to);

/**
  * The undo_rename_cancel function is used to cancel an undo request
  * submitted with undo_rename().
  *
  * \param from
  *     path of file now
  * \param to
  *     path of file_after rollback.
  */
void undo_rename_cancel(const nstring &from, const nstring &to);

/**
  * The undo_chmod function is used to put a chmod operation in the undo queue.
  *
  * \param path
  *     The path to have its mode changed.
  * \param mode
  *     The mod to change the path to.
  */
void undo_chmod(string_ty *path, int mode);

/**
  * The undo_chmod function is used to put a chmod operation in the undo queue.
  *
  * \param path
  *     The path to have its mode changed.
  * \param mode
  *     The mod to change the path to.
  */
void undo_chmod(const nstring &path, int mode);

/**
  * The undo_chmod function is used to put a chmod operation in the undo
  * queue.  Errors are ignored.
  *
  * \param path
  *     The path to have its mode changed.
  * \param mode
  *     The mod to change the path to.
  */
void undo_chmod_errok(string_ty *path, int mode);

/**
  * The undo_chmod_errok function is used to put a chmod operation in
  * the undo queue.  Errors are ignored.
  *
  * \param path
  *     The path to have its mode changed.
  * \param mode
  *     The mod to change the path to.
  */
void undo_chmod_errok(const nstring &path, int mode);

/**
  * The undo_unlink_errok function si sued to put an unlink operation in
  * the undo queue.
  *
  * \param path
  *     The path opf the file to be removed.
  */
void undo_unlink_errok(string_ty *path);

/**
  * The undo_unlink_errok function si sued to put an unlink operation in
  * the undo queue.
  *
  * \param path
  *     The path opf the file to be removed.
  */
void undo_unlink_errok(const nstring &path);

/**
  * The undo_rmdir_bg function is used to put a rmdir (recursive
  * directory remove) operation in the undo queue.  The operation is
  * performed in the background.
  *
  * \param path
  *     The path of the directory to be removed.
  */
void undo_rmdir_bg(string_ty *path);

/**
  * The undo_rmdir_bg function is used to put a rmdir (recursive
  * directory remove) operation in the undo queue.  The operation is
  * performed in the background.
  *
  * \param path
  *     The path of the directory to be removed.
  */
void undo_rmdir_bg(const nstring &path);

/**
  * The undo_rmdir_errok function is used to put a rmdir (non-recursive)
  * operation in the undo queue.  Any error (e.g. directory not empty)
  * will be ignored.
  *
  * \param path
  *     The path of the directory to be removed.
  */
void undo_rmdir_errok(string_ty *path);

/**
  * The undo_rmdir_errok function is used to put a rmdir (non-recursive)
  * operation in the undo queue.  Any error (e.g. directory not empty)
  * will be ignored.
  *
  * \param path
  *     The path of the directory to be removed.
  */
void undo_rmdir_errok(const nstring &path);

/**
  * The undo_message function is used to print a message in the event of
  * failure.
  *
  * \param msg
  *     The message to be printed.
  */
void undo_message(string_ty *msg);

/**
  * The undo_message function is used to print a message in the event of
  * failure.
  *
  * \param msg
  *     The message to be printed.
  */
void undo_message(const nstring &msg);

/**
  * The undo function is used to pun all pending operation in the undo queue.
 */
void undo(void);

extern quit_action_undo undo_quitter;

/**
  * The undo_cancel function is used to cancel any pending operation
  * from the undo queue.
  */
void undo_cancel(void);

/** @} */
#endif // UNDO_H
