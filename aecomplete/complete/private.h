/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: interface definition for aecomplete/complete/private.c
 */

#ifndef AECOMPLETE_COMPLETE_PRIVATE_H
#define AECOMPLETE_COMPLETE_PRIVATE_H

#include <complete.h>

typedef struct complete_vtbl_ty complete_vtbl_ty;
struct complete_vtbl_ty
{
    void (*destructor)_((complete_ty *));
    void (*perform)_((complete_ty *, struct shell_ty *));
    int size;
    char *name;
};

complete_ty *complete_new _((complete_vtbl_ty *));

#endif /* AECOMPLETE_COMPLETE_PRIVATE_H */