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

#include <common/error.h> // for assert
#include <aeimport/format/version.h>
#include <aeimport/format/version_list.h>
#include <common/mem.h>


format_version_ty::format_version_ty() :
    filename_physical(0),
    filename_logical(0),
    edit(0),
    when(0),
    who(0),
    description(0),
    before(0),
    after(0),
    after_branch(0),
    dead(0)
{
}


format_version_ty::~format_version_ty()
{
    if (filename_physical)
    {
        str_free(filename_physical);
        filename_physical = 0;
    }
    if (filename_logical)
    {
        str_free(filename_logical);
        filename_logical = 0;
    }
    if (edit)
    {
        str_free(edit);
        edit = 0;
    }
    when = 0;
    if (who)
    {
        str_free(who);
        who = 0;
    }
    if (description)
    {
        str_free(description);
        description = 0;
    }
    before = 0;
    if (after)
    {
        delete(after);
        after = 0;
    }
    if (after_branch)
    {
        format_version_list_delete(after_branch, 1);
        after_branch = 0;
    }
    dead = 0;
}


#ifdef DEBUG

void
format_version_ty::validate()
    const
{
    if (filename_physical)
        assert(str_validate(filename_physical));
    if (filename_logical)
        assert(str_validate(filename_logical));
    if (edit)
        assert(str_validate(edit));
    if (who)
        assert(str_validate(who));
    if (description)
        assert(str_validate(description));
    assert(tag.validate());
    if (after)
        after->validate();
    if (after_branch)
        format_version_list_validate(after_branch);
}

#endif
