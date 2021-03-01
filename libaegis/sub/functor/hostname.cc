//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#include <common/ac/unistd.h>
#include <sys/utsname.h>

#include <common/wstring/list.h>
#include <libaegis/sub.h>
#include <libaegis/sub/functor/hostname.h>


sub_functor_hostname::~sub_functor_hostname()
{
}


sub_functor_hostname::sub_functor_hostname(const nstring &aname) :
    sub_functor(aname)
{
}


sub_functor::pointer
sub_functor_hostname::create(const nstring &aname)
{
    return pointer(new sub_functor_hostname(aname));
}


wstring
sub_functor_hostname::evaluate(sub_context_ty *scp, const wstring_list &arg)
{
    if (arg.size() != 1)
    {
        scp->error_set(i18n("requires zero arguments"));
    }
    else
    {
        char buf1[300];
        if (gethostname(buf1, sizeof(buf1)) >= 0)
            return wstring(buf1);
        struct utsname buf2;
        if (uname(&buf2) >= 0)
            return wstring(buf2.nodename);
    }
    return wstring();
}
