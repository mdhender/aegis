//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
//	Copyright (C) 2005 Walter Franzini;
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <libaegis/ael/column_width.h>
#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <aedist/change/functor/pendin_print.h>
#include <libaegis/col.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <common/symtab/template.h>

change_functor_pending_printer::~change_functor_pending_printer()
{
    if (pp)
        project_free(pp);
    size_t sz = remote_inventory->size();
    uuid_col->fprintf("Remote change set%s: %d.", sz == 1 ? "" : "s", (int) sz);
    colp->eoln();
    uuid_col->fprintf("Pending %d change set%s.", n, (n == 1 ? "" : "s"));
    colp->eoln();
}


change_functor_pending_printer::change_functor_pending_printer(bool arg1,
	project_ty *arg2, const nstring &arg3, symtab<nstring> *arg4,
	nstring_list *arg5, nstring_list *arg6, nstring_list *arg7,
	nstring_list *arg8) :
    change_functor(arg1),
    pp(arg2),
    remote_inventory(arg4),
    in_uuid_list(arg5),
    in_version_list(arg6),
    ex_uuid_list(arg7),
    ex_version_list(arg8),
    n(0)
{
    colp = col::open(0);
    nstring line1 =
        nstring::format
        (
            "Project \"%s\", Pending Change Set Inventory",
            project_name_get(pp)->str_text
        );
    colp->title(line1.c_str(), arg3.c_str());

    int left = 0;
    vers_col =
        colp->create(left, left + VERSION_WIDTH, "Change\n------");
    left += VERSION_WIDTH + 1;
    uuid_col =
        colp->create(left, left + UUID_WIDTH, "UUID\n------");
    left += UUID_WIDTH + 1;
    desc_col =
        colp->create(left, 0, "Description\n-----------");
}



void
change_functor_pending_printer::operator()(change::pointer cp)
{
    cstate_ty *cstate_data = cp->cstate_get();

    if (cstate_data->uuid)
    {
        if (!remote_inventory->query(cstate_data->uuid))
        {
            nstring version(change_version_get(cp));
            nstring uuid(cstate_data->uuid);

            if (ex_uuid_list->member(uuid))
                return;
            if (ex_version_list->gmatch_candidate(version))
                return;

            if
            (
                !in_uuid_list->empty()
            &&
                !in_uuid_list->member(uuid)
            )
                return;
            if
            (
                !in_version_list->empty()
            &&
                !in_version_list->gmatch_candidate(version)
            )
                return;

            vers_col->fputs(version);
            uuid_col->fputs(uuid);
            desc_col->fputs(nstring(cstate_data->brief_description));
            colp->eoln();
            ++n;
        }
    }
}
