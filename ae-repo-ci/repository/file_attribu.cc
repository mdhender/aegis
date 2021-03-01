//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/nstring.h>
#include <libaegis/fstate.fmtgen.h>

#include <ae-repo-ci/repository.h>


void
repository::file_attributes(change_identifier &, fstate_src_ty *src)
{
    nstring filename(src->file_name);
    attributes_list_ty *alp = src->attribute;
    if (alp)
    {
        for (size_t j = 0; j < alp->length; ++j)
        {
            attributes_ty *ap = alp->list[j];
            assert(ap);
            assert(ap->name);
            assert(ap->value);
            nstring attribute_name(ap->name);
            nstring attribute_value(ap->value);
            file_attribute(filename, attribute_name, attribute_value);
        }
    }

    nstring attribute_name("executable");
    nstring attribute_value(src->executable ? "true" : "false");
    file_attribute(filename, attribute_name, attribute_value);
}


// vim: set ts=8 sw=4 et :
