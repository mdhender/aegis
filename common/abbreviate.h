/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: interface definition for common/abbreviate.c
 */

#ifndef COMMON_ABBREVIATE_H
#define COMMON_ABBREVIATE_H

#include <main.h>

struct string_ty *abbreviate_filename _((struct string_ty *, int));
struct string_ty *abbreviate_dirname _((struct string_ty *, int));
struct string_ty *abbreviate_8dos3 _((struct string_ty *));

#endif /* COMMON_ABBREVIATE_H */