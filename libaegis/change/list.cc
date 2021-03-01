//
//	aegis - project change supervisor
//	Copyright (C) 2003-2008 Peter Miller
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

#include <common/error.h>
#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/list.h>
#include <libaegis/project.h>


change_list_ty::change_list_ty()
{
    trace(("change_list_ty::change_list_ty()\n{\n"));
    length = 0;
    maximum = 0;
    item = 0;
    trace(("}\n"));
}


change_list_ty::~change_list_ty()
{
    trace(("change_list_ty::~change_list_ty(this = %08lX)\n{\n", (long)this));
    delete [] item;
    length = 0;
    maximum = 0;
    item = 0;
    trace(("}\n"));
}


void
change_list_ty::clear()
{
    trace(("change_list::clear(this = %08lX)\n{\n", (long)this));
    length = 0;
    trace(("}\n"));
}


void
change_list_ty::append(change::pointer cp)
{
    trace(("change_list_ty::append(this = %08lX, cp = %08lX)\n{\n", (long)this,
	(long)cp));
    assert(cp);
    trace(("project \"%s\", change %ld, delta %ld\n",
	project_name_get(cp->pp)->str_text, magic_zero_decode(cp->number),
	change_delta_number_get(cp)));
    if (length >= maximum)
    {
	maximum = maximum * 2 + 8;
	change::pointer *new_item = new change::pointer  [maximum];
	for (size_t k = 0; k < length; ++k)
	    new_item[k] = item[k];
	delete [] item;
	item = new_item;
    }
    item[length++] = cp;
    trace(("}\n"));
}


bool
change_list_ty::member_p(change::pointer cp1)
    const
{
    size_t          j;

    for (j = 0; j < length; ++j)
    {
	change::pointer cp2 = item[j];
	if (cp1 == cp2)
	    return true;
	if
	(
	    cp1->number == cp2->number
	&&
	    str_equal(project_name_get(cp1->pp), project_name_get(cp2->pp))
	)
	    return true;
    }
    return false;
}
