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
 * MANIFEST: interface definition for fmtgen/id.c
 */

#ifndef ID_H
#define ID_H

#include <main.h>
#include <str.h>
#include <word.h>

enum id_class_ty
{
    ID_CLASS_KEYWORD,
    ID_CLASS_TYPE,
    ID_CLASS_FIELD,
    ID_CLASS_ENUMEL
};
typedef enum id_class_ty id_class_ty;

typedef int (*bifp)_((wlist *, wlist *));

void id_initialize _((void));
int id_search _((string_ty *, id_class_ty, long *));
void id_assign _((string_ty *, id_class_ty, long));
void id_dump _((char *, int));

#endif /* ID_H */
