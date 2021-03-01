//
// aegis - project change supervisor
// Copyright (C) 2004-2006, 2008, 2011, 2012 Peter Miller
// Copyright (C) 2006, 2007, 2009 Walter Franzini
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/itab.h>
#include <common/symtab/template.h>
#include <common/trace.h>
#include <libaegis/project/file.h>


fstate_src_ty *
project::file_find(fstate_src_ty *c_src_data, view_path_ty vp)
{
    fstate_src_ty   *p_src_data;

    trace(("project::file_find(c_src = %p, vp = %s)\n{\n", c_src_data,
        view_path_ename(vp)));
    trace
    ((
        "change: %s %s \"%s\" %s %s\n", file_usage_ename(c_src_data->usage),
        file_action_ename(c_src_data->action),
        c_src_data->file_name->str_text,
        (c_src_data->edit_origin ?
            c_src_data->edit_origin->revision->str_text : ""),
        (c_src_data->edit ?
            c_src_data->edit->revision->str_text : "")
    ));

    //
    // Try to find the file by its UUID.
    //
    // For backwards compatibility reasons (UUIDs were introduced in
    // 4.17) there could be a rename chain (pointing at UUID-less
    // entries) to be taken into account.
    //
    if (c_src_data->uuid)
    {
        itab_ty *found = itab_alloc();
        p_src_data = file_find_by_uuid(c_src_data->uuid, vp);
        while (p_src_data && !itab_query(found, (itab_key_ty)p_src_data))
        {
            if (p_src_data->action != file_action_remove || !p_src_data->move)
            {
                trace
                ((
                    "project: %s %s \"%s\" %s %s\n",
                    file_usage_ename(p_src_data->usage),
                    file_action_ename(p_src_data->action),
                    p_src_data->file_name->str_text,
                    (p_src_data->edit_origin ?
                        p_src_data->edit_origin->revision->str_text : ""),
                    (p_src_data->edit ?
                        p_src_data->edit->revision->str_text : "")
                ));
                itab_free(found);
                trace(("return %p;\n", p_src_data));
                trace(("}\n"));
                return p_src_data;
            }
            nstring mname(p_src_data->move);
            p_src_data = file_find(mname, vp);
            itab_assign(found, (itab_key_ty)p_src_data, p_src_data);
        }
        itab_free(found);
    }

    //
    // Look for the file by name, tracking renames as they happen.
    //
    nstring mname;
    if (c_src_data->action == file_action_create && c_src_data->move)
        mname = nstring(c_src_data->move);
    else
        mname = nstring(c_src_data->file_name);

    itab_ty *found = itab_alloc();
    for (;;)
    {
        p_src_data = file_find(mname, vp);
        if (!p_src_data)
        {
            itab_free(found);
            trace(("return NULL;\n"));
            trace(("}\n"));
            return 0;
        }

        //
        // A loop has been detected go away.
        //
        if (itab_query(found, (itab_key_ty)p_src_data))
        {
            itab_free(found);
            trace(("return NULL;\n"));
            trace(("}\n"));
            return 0;
        }
        itab_assign(found, (itab_key_ty)p_src_data, p_src_data);

        if (p_src_data->action != file_action_remove || !p_src_data->move)
        {
            trace
            ((
                "project: %s %s \"%s\" %s %s\n",
                file_usage_ename(p_src_data->usage),
                file_action_ename(p_src_data->action),
                p_src_data->file_name->str_text,
                (p_src_data->edit_origin ?
                    p_src_data->edit_origin->revision->str_text : ""),
                (p_src_data->edit ?
                    p_src_data->edit->revision->str_text : "")
            ));
            itab_free(found);
            trace(("return %p;\n", p_src_data));
            trace(("}\n"));
            return p_src_data;
        }
        mname = nstring(p_src_data->move);
    }
}


// vim: set ts=8 sw=4 et :
