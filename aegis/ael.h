/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: interface definition for aegis/ael.c
 */

#ifndef AEGIS_AEL_H
#define AEGIS_AEL_H

#include <main.h>

struct string_ty;

void list _((void));

void list_administrators _((struct string_ty *, long));
void list_change_details _((struct string_ty *, long));
void list_change_files _((struct string_ty *, long));
void list_change_history _((struct string_ty *, long));
void list_changes_in_state_mask _((struct string_ty *, int state_mask));
void list_changes _((struct string_ty *, long));
void list_default_change _((struct string_ty *, long));
void list_default_project _((struct string_ty *, long));
void list_developers _((struct string_ty *, long));
void list_integrators _((struct string_ty *, long));
void list_list_list _((struct string_ty *, long));
void list_locks _((struct string_ty *, long));
void list_outstanding_changes _((struct string_ty *, long));
void list_outstanding_changes_all _((struct string_ty *, long));
void list_project_files _((struct string_ty *, long));
void list_project_history _((struct string_ty *, long));
void list_projects _((struct string_ty *, long));
void list_reviewers _((struct string_ty *, long));
void list_user_changes _((struct string_ty *, long));
void list_version _((struct string_ty *, long));

#endif /* AEGIS_AEL_H */
