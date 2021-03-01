//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
//	All rights reserved.
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
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: implementation of the change_functor_inventory_list class
//

#pragma implementation "change_functor_inventory_list"

#include <ac/string.h>

#include <ael/column_width.h>
#include <attribute.h>
#include <change/branch.h>
#include <change/functor/invento_list.h>
#include <col.h>
#include <error.h> // for assert
#include <option.h>
#include <output.h>
#include <project.h>
#include <uuidentifier.h>


change_functor_inventory_list::~change_functor_inventory_list()
{
    col_close(colp);
}


change_functor_inventory_list::change_functor_inventory_list(project_ty *pp) :
    colp(0),
    vers_col(0),
    uuid_col(0),
    when_col(0)
{
    colp = col_open((string_ty *)0);
    string_ty *line1 =
	str_format("Project \"%s\"", project_name_get(pp)->str_text);
    col_title(colp, line1->str_text, "Change Set Inventory");
    str_free(line1);
    line1 = 0;

    int left = 0;
    vers_col = col_create(colp, left, left + VERSION_WIDTH, "Change\n-------");
    left += VERSION_WIDTH + 1;
    uuid_col = col_create(colp, left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;
    if (option_verbose_get())
	when_col = col_create(colp, left, 0, "Date and Time\n---------------");
}


void
change_functor_inventory_list::print_one_line(change_ty *cp, string_ty *uuid)
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
    col_eoln(colp);
}


void
change_functor_inventory_list::operator()(change_ty *cp)
{
    cstate_ty *cstate_data = change_cstate_get(cp);
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
