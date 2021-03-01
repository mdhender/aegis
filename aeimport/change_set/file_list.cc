//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#include <aeimport/change_set/file_list.h>
#include <common/error.h> // for assert


change_set_file_list_ty::change_set_file_list_ty() :
    length(0),
    maximum(0),
    item(0)
{
}


change_set_file_list_ty::~change_set_file_list_ty()
{
    clear();
    if (item)
        delete item;
    item = 0;
    maximum = 0;
}


#ifdef DEBUG

void
change_set_file_list_ty::validate()
    const
{
    assert(length <= maximum);
    assert(!item == !maximum);
    for (size_t j = 0; j < length; ++j)
        item[j]->validate();
}

#endif


void
change_set_file_list_ty::push_back(change_set_file_ty *csfp)
{
    //
    // If we already have the file, just update the edit number.
    //
    for (size_t j = 0; j < length; ++j)
    {
        change_set_file_ty *csfp2 = item[j];
        if (str_equal(csfp->filename, csfp2->filename))
        {
	    csfp2->merge(*csfp);
	    delete csfp;
            return;
        }
    }

    if (length >= maximum)
    {
        size_t new_maximum = maximum * 2 + 4;
        change_set_file_ty **new_item = new change_set_file_ty * [new_maximum];
	for (size_t k = 0; k < length; ++k)
	    new_item[k] = item[k];
	delete item;
	maximum = new_maximum;
	item = new_item;
    }
    item[length++] = csfp;
}


void
change_set_file_list_ty::clear()
{
    while (length)
	delete item[--length];
}
