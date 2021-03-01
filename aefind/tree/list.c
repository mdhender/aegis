/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate tree node lists
 */

#include <mem.h>
#include <tree.h>
#include <tree/list.h>


static void
tree_list_constructor(tree_list_ty *tlp)
{
    tlp->length = 0;
    tlp->maximum = 0;
    tlp->item = 0;
}


tree_list_ty *
tree_list_new(void)
{
    tree_list_ty    *tlp;

    tlp = (tree_list_ty *)mem_alloc(sizeof(tree_list_ty));
    tree_list_constructor(tlp);
    return tlp;
}


static void
tree_list_destructor(tree_list_ty *tlp)
{
    size_t	    j;

    for (j = 0; j < tlp->length; ++j)
	tree_delete(tlp->item[j]);
    if (tlp->item)
	mem_free(tlp->item);
    tlp->length = 0;
    tlp->maximum = 0;
    tlp->item = 0;
}


void
tree_list_delete(tree_list_ty *tlp)
{
    tree_list_destructor(tlp);
    mem_free(tlp);
}


void
tree_list_append(tree_list_ty *tlp, tree_ty *tp)
{
    if (tlp->length >= tlp->maximum)
    {
	size_t		nbytes;

	tlp->maximum = tlp->maximum * 2 + 4;
	nbytes = tlp->maximum * sizeof(tlp->item[0]);
	tlp->item = (tree_ty **)mem_change_size(tlp->item, nbytes);
    }
    tlp->item[tlp->length++] = tree_copy(tp);
}
