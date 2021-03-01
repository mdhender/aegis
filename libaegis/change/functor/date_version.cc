//
// aegis - project change supervisor
// Copyright (C) 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/nstring/list.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/functor/date_version.h>


change_functor_date_version::~change_functor_date_version()
{
}


change_functor_date_version::change_functor_date_version(nstring_list &arg) :
    change_functor(true),
    versions(arg)
{
}


void
change_functor_date_version::operator()(change::pointer cp)
{
    time_t when = cp->completion_timestamp();
    struct tm *tmp = gmtime(&when);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y.%m.%d.%H.%M.%S", tmp);
    nstring s1(buffer);
    nstring s2 = cp->version_get();
    versions.push_back(s1 + "/" + s2);
}


// vim: set ts=8 sw=4 et :
