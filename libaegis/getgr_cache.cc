//
//      aegis - project change supervisor
//      Copyright (C) 2001-2008, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/itab.h>
#include <common/mem.h>
#include <common/symtab.h>
#include <libaegis/getgr_cache.h>

static symtab_ty *name_table;
static itab_ty *gid_table;


static group *
group_null(void)
{
    group *result = new group;

    //
    // This isn't portable, it assumes that NULL pointers are
    // all-bits-zero, but this is only to cover all the fields we
    // don't care about.  We set the ones we -do- care about to
    // NULL explicitly.
    //
    memset(result, 0, sizeof(*result));

#ifndef SOURCE_FORGE_HACK
    result->gr_name = 0;
    result->gr_passwd = 0;
    result->gr_gid = 0;
    result->gr_mem = 0;
#else
    //
    // This is used to fake a group, because on SourceForge.net
    // the Apache servers don't have access to /etc/group.  The fake
    // information will be further taylored below.
    //
    result->gr_name = "nogroup";
    result->gr_passwd = "*";
    result->gr_gid = AEGIS_MIN_GID;
    result->gr_mem = 0;
#endif

    //
    // All done.
    //
    return result;
}


static group *
group_copy(group *gr)
{
    group *result = group_null();

    //
    // Copy values onto the heap, because the next call to getgrnam
    // will trash the ones in *gr.
    //
    result->gr_name = mem_copy_string(gr->gr_name);
    result->gr_gid = gr->gr_gid;

    //
    // All done.
    //
    return result;
}


group *
getgrnam_cached(string_ty *name)
{
    return getgrnam_cached(nstring(name));
}


group *
getgrnam_cached(const nstring &name)
{
    //
    // Create the tables if they don't exist already.
    //
    if (!name_table)
    {
        name_table = new symtab_ty(5);
        gid_table = itab_alloc();
    }

    //
    // Look for the data in the name table.
    //
    group *data = (group *)name_table->query(name);

    //
    // If the data isn't there, ask the system for it.
    //
    if (!data)
    {
        group *gr = getgrnam(name.c_str());
        if (gr)
        {
            data = group_copy(gr);
            itab_assign(gid_table, data->gr_gid, data);
        }
        else
        {
            data = group_null();
#ifdef SOURCE_FORGE_HACK
            data->gr_name = mem_copy_string(name.c_str());
#endif
        }
        name_table->assign(name, data);
    }

    //
    // Negative search results are also cached.
    // They have NULL pointers for all the fields.
    //
    if (!data->gr_name)
        return 0;

    //
    // Success.
    //
    return data;
}


group *
getgrgid_cached(int gid)
{
    //
    // Create the tables if they don't exist already.
    //
    if (!name_table)
    {
        name_table = new symtab_ty(5);
        gid_table = itab_alloc();
    }

    //
    // Look for the data in the name table.
    //
    group *data = (group *)itab_query(gid_table, gid);

    //
    // If the data isn't there, ask the system for it.
    //
    if (!data)
    {
        group *gr = getgrgid(gid);
        if (gr)
        {
            nstring name(gr->gr_name);
            data = group_copy(gr);
            name_table->assign(name, data);
        }
        else
        {
            data = group_null();
            data->gr_gid = gid;
        }
        itab_assign(gid_table, gid, data);
    }

    //
    // Negative search results are also cached.
    // They have NULL pointers for all the fields.
    //
    if (!data->gr_name)
        return 0;

    //
    // Success.
    //
    return data;
}


// vim: set ts=8 sw=4 et :
