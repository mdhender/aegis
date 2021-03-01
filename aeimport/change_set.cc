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

#include <aeimport/change_set.h>
#include <common/error.h> // for assert


change_set_ty::change_set_ty()
{
    who = 0;
    when = 0;
    description = 0;
    change_set_file_list_constructor(&file);
}


change_set_ty::~change_set_ty()
{
    if (who)
    {
        str_free(who);
        who = 0;
    }
    when = 0;
    if (description)
    {
        str_free(description);
        description = 0;
    }
    change_set_file_list_destructor(&file);
}


#ifdef DEBUG

void
change_set_ty::validate()
    const
{
    assert(!who || str_validate(who));
    assert(!description || str_validate(description));
    file.validate();
    assert(tag.validate());
}

#endif
