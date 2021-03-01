//
//	aegis - project change supervisor
//	Copyright (C) 2003-2006, 2008 Peter Miller
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

#ifndef AEDIST_MOVE_H
#define AEDIST_MOVE_H

#include <common/str.h>


typedef struct move_ty move_ty;
struct move_ty
{
    string_ty       *from;
    string_ty       *to;
    int             create;
    int             remove;
};

/**
  * The move_constructor function is used to initialize a move structure.
  */
void move_constructor(move_ty *mp, string_ty *, string_ty *);

/**
  * The move_destructor function is used to release the resources held
  * by a move structure.
  */
void move_destructor(move_ty *mp);


typedef struct move_list_ty move_list_ty;
struct move_list_ty
{
    size_t          length;
    size_t          maximum;
    move_ty         *item;
};

/**
  * The move_list_constructor function is used to initialize a move
  * list structure.
  */
void move_list_constructor(move_list_ty *mlp);

/**
  * The move_list_destructor function is used to release the resources
  * held by a move list structure.
  */
void move_list_destructor(move_list_ty *mlp);

/**
  * The move_list_append function is used to append a (from, to) pair
  * to a move list.  The fact that it was the create half is recorded.
  */
void move_list_append_create(move_list_ty *, string_ty *, string_ty *);

/**
  * The move_list_append function is used to append a (from, to) pair
  * to a move list.  The fact that it was the remove half is recorded.
  */
void move_list_append_remove(move_list_ty *, string_ty *, string_ty *);

#endif // AEDIST_MOVE_H
