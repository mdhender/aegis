//
//      aegis - project change supervisor
//      Copyright (C) 2003-2006, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/mem.h>
#include <libaegis/move_list.h>


void
move_constructor(move_ty *mp, string_ty *from, string_ty *to)
{
    mp->from = str_copy(from);
    mp->to = str_copy(to);
    mp->create = 0;
    mp->remove = 0;
}


void
move_destructor(move_ty *mp)
{
    str_free(mp->from);
    str_free(mp->to);
}


void
move_list_constructor(move_list_ty *mlp)
{
    mlp->length = 0;
    mlp->maximum = 0;
    mlp->item = 0;
}


void
move_list_destructor(move_list_ty *mlp)
{
    size_t          j;

    for (j = 0; j < mlp->length; ++j)
        move_destructor(mlp->item + j);
    delete [] mlp->item;
    mlp->length = 0;
    mlp->maximum = 0;
    mlp->item = 0;
}


static move_ty *
move_list_append_unique(move_list_ty *mlp, string_ty *from, string_ty *to)
{
    size_t          j;
    move_ty         *mp;

    for (j = 0; j < mlp->length; ++j)
    {
        mp = mlp->item + j;
        if (str_equal(mp->from, from) && str_equal(mp->to, to))
            return mp;
    }
    if (mlp->length >= mlp->maximum)
    {
        mlp->maximum = mlp->maximum * 2 + 4;
	move_ty *new_item = new move_ty [mlp->maximum];
	for (size_t k = 0; k < mlp->length; ++k)
	    new_item[k] = mlp->item[k];
	delete [] mlp->item;
	mlp->item = new_item;
    }
    mp = mlp->item + mlp->length++;
    move_constructor(mp, from, to);
    return mp;
}


void
move_list_append_create(move_list_ty *mlp, string_ty *from, string_ty *to)
{
    move_ty         *mp;

    mp = move_list_append_unique(mlp, from, to);
    mp->create = 1;
}


void
move_list_append_remove(move_list_ty *mlp, string_ty *from, string_ty *to)
{
    move_ty         *mp;

    mp = move_list_append_unique(mlp, from, to);
    mp->remove = 1;
}
