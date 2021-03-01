/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 2002, 2004 Peter Miller.
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
 * MANIFEST: interface definition for aegis/undo.c
 */

#ifndef UNDO_H
#define UNDO_H

#include <quit/action/undo.h>
#include <str.h>

/** \addtogroup Transaction
  * \ingroup AegisLibrary
  * @{
  */

/** The undo_rename funtion is used to submit an undo request
  * (rollback) to rename a file.
  *
  * \param from
  *      path of file now
  * \param to
  *      path of file after rollback.
  */
void undo_rename(string_ty *from, string_ty *to);

/** The undo_rename_cancel function is used to cancel an undo request
  * submitted with undo_rename().
  *
  * \param from
  *     path of file now
  * \param to
  *     path of file_after rollback.
  */
void undo_rename_cancel(string_ty *from, string_ty *to);

/** Put a chmod operation in the undo queue.
 */
void undo_chmod(string_ty *path, int mode);
void undo_chmod_errok(string_ty *path, int mode);

/** Put an unlink operation in the undo queue.
 */
void undo_unlink_errok(string_ty *path);

/** Put a rmdir operation in the undo queue.
 */
void undo_rmdir_bg(string_ty *path);
void undo_rmdir_errok(string_ty *path);

void undo_message(string_ty *);

/** Run all pending operation in the undo queue.
 */
void undo(void);

extern quit_action_undo undo_quitter;

/** Cancel any pending operation from the undo queue.
 */
void undo_cancel(void);

/** @} */
#endif /* UNDO_H */
