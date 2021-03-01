//
//	aegis - project change supervisor
//	Copyright (C) 2004-2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
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

#include <common/ac/string.h>

#include <libaegis/ael/column_width.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/functor/invento_list.h>
#include <libaegis/col.h>
#include <common/error.h> // for assert
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <common/uuidentifier.h>


change_functor_inventory_list::~change_functor_inventory_list()
{
    delete colp;
}


change_functor_inventory_list::change_functor_inventory_list(bool arg1,
	project_ty *pp) :
    change_functor(arg1),
    colp(0),
    vers_col(0),
    uuid_col(0),
    when_col(0)
{
    colp = col::open((string_ty *)0);
    string_ty *line1 =
	str_format("Project \"%s\"", project_name_get(pp)->str_text);
    colp->title(line1->str_text, "Change Set Inventory");
    str_free(line1);
    line1 = 0;

    int left = 0;
    vers_col = colp->create(left, left + VERSION_WIDTH, "Change\n-------");
    left += VERSION_WIDTH + 1;
    uuid_col = colp->create(left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;
    if (option_verbose_get())
	when_col = colp->create(left, 0, "Date and Time\n---------------");
}


void
change_functor_inventory_list::print_one_line(change::pointer cp,
    string_ty *uuid)
{
    vers_col->fputs(change_version_get(cp));
    uuid_col->fputs(uuid);
    if (when_col)
    {
	//
	// Change UUIDs were introduces in 4.16 (public release
	// 4.17), so for backwards compatibility reasons, do not
	// expect all completed changes have UUIDs.
	//
	time_t when = change_completion_timestamp(cp);
	struct tm *theTm = localtime(&when);
	char buffer[30];
	strftime(buffer, sizeof(buffer), "%Y-%b-%d %H:%M:%S", theTm);
	when_col->fputs(buffer);
    }
    colp->eoln();
}


void
change_functor_inventory_list::operator()(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();
    if (cstate_data->uuid)
	print_one_line(cp, cstate_data->uuid);
    if (!cstate_data->attribute)
	return;
    for (size_t j = 0; j < cstate_data->attribute->length; ++j)
    {
	attributes_ty *ap = cstate_data->attribute->list[j];
	assert(ap->name);
	assert(ap->value);
	if
	(
	    ap->value
	&&
	    // users can edit, we will check
	    universal_unique_identifier_valid(ap->value)
	)
	{
	    print_one_line(cp, ap->value);
	}
    }
}
