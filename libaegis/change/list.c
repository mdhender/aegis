/*
 *	aegis - project change supervisor
 *	Copyright (C) 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate lists
 */

#include <change.h>
#include <change/list.h>
#include <error.h>
#include <mem.h>
#include <project.h>
#include <trace.h>


change_list_ty *
change_list_new(void)
{
    change_list_ty *result;

    trace(("change_list_new()\n{\n"));
    result = mem_alloc(sizeof(change_list_ty));
    result->length = 0;
    result->maximum = 0;
    result->item = 0;
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


void
change_list_delete(change_list_ty *clp)
{
    trace(("change_list_delete(clp = %08lX)\n{\n", (long)clp));
    if (clp->item)
	mem_free(clp->item);
    clp->length = 0;
    clp->maximum = 0;
    clp->item = 0;
    mem_free(clp);
    trace(("}\n"));
}


void
change_list_append(change_list_ty *clp, change_ty *cp)
{
    trace(("change_list_append(clp = %08lX, cp = %08lX)\n{\n", (long)clp,
	(long)cp));
    assert(cp);
    trace(("project \"%s\", change %ld, delta %ld\n",
	project_name_get(cp->pp)->str_text, magic_zero_decode(cp->number),
	change_delta_number_get(cp)));
    if (clp->length >= clp->maximum)
    {
	size_t          nbytes;

	clp->maximum = clp->maximum * 2 + 8;
	nbytes = clp->maximum * sizeof(clp->item[0]);
	clp->item = mem_change_size(clp->item, nbytes);
    }
    clp->item[clp->length++] = cp;
    trace(("}\n"));
}


int
change_list_member_p(change_list_ty *clp, change_ty *cp1)
{
    size_t          j;

    for (j = 0; j < clp->length; ++j)
    {
	change_ty       *cp2;

	cp2 = clp->item[j];
	if (cp1 == cp2)
	    return 1;
	if
	(
	    cp1->number == cp2->number
	&&
	    str_equal(project_name_get(cp1->pp), project_name_get(cp2->pp))
	)
	    return 1;
    }
    return 0;
}
