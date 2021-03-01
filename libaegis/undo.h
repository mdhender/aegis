/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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

#include <str.h>

void undo_rename _((string_ty *from, string_ty *to));
void undo_chmod _((string_ty *path, int mode));
void undo_chmod_errok _((string_ty *path, int mode));
void undo_unlink_errok _((string_ty *path));
void undo_rmdir_bg _((string_ty *path));
void undo_rmdir_errok _((string_ty *path));
void undo_message _((string_ty *));
void undo _((void));
void undo_quitter _((int));
void undo_cancel _((void));

#endif /* UNDO_H */
