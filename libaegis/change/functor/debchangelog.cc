//
// aegis - project change supervisor
// Copyright (C) 2008, 2009, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/time.h>

#include <common/nstring/list.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/functor/debchangelog.h>
#include <libaegis/project.h>


change_functor_debchangelog::~change_functor_debchangelog()
{
}


change_functor_debchangelog::change_functor_debchangelog(col::pointer a_colp) :
    change_functor(true),
    colp(a_colp)
{
    heading = colp->create(0, 0, "");
    signature = colp->create(1, 3, "");
    star = colp->create(2, 3, "");
    description = colp->create(4, 0, "");
}


static nstring
safe_strftime(const char *fmt, time_t when)
{
    char buffer[100];
    struct tm *tmp = localtime(&when);
    assert(tmp);
    strftime(buffer, sizeof(buffer), fmt, tmp);
    return buffer;
}


void
change_functor_debchangelog::operator()(change::pointer cp)
{
    //
    // Aegis' branch numbering confuses dpkg-buildpackage, so only tell
    // it about change sets, and don't tell it about branches.
    //
    if (!cp->was_a_branch())
        print(cp);
}


void
change_functor_debchangelog::print(change::pointer cp)
{
    nstring_list empty;
    print(cp, empty);
}


void
change_functor_debchangelog::print(change::pointer cp,
    const nstring_list &extra)
{
    heading->fprintf
    (
        "%s (%s-1) unstable; urgency=low",
        cp->pp->trunk_get()->name_get()->str_text,
        cp->version_debian_get().c_str()
    );
    colp->eoln();
    colp->eoln();

    nstring desc(cp->description_get());
    if (desc.empty())
        desc = "No commit comment.";
    desc = desc.substr(0, 2000);
    nstring_list desc_list;
    desc_list.split(desc, "\n");
    for (size_t j = 0; j < desc_list.size(); ++j)
    {
        nstring tmp2 = desc_list[j];
        if (!tmp2.empty())
        {
            star->fputs("*");
            description->fputs(tmp2);
            colp->eoln();
        }
    }
    for (size_t j = 0; j < extra.size(); ++j)
    {
        nstring tmp2 = extra[j];
        if (!tmp2.empty())
        {
            star->fputs("*");
            description->fputs(tmp2);
            colp->eoln();
        }
    }
    colp->eoln();

    nstring dev(cp->developer_name());
    time_t when = cp->completion_timestamp();
    nstring when_str = safe_strftime("%a, %d %b %Y %H:%M:%S %z", when);
    user_ty::pointer up = user_ty::create(dev);
    nstring email = up->get_email_address();
    if (email[0] == '<')
        email = dev + " " + email;
    signature->fputs("--");
    description->fputs(email + "  " + when_str);
    colp->eoln();
    colp->eoln();
}


// vim: set ts=8 sw=4 et :
