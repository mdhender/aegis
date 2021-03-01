//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
//      Copyright (C) 2008 Walter Franzini
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

#include <common/ac/assert.h>

#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/ael/attribu_list.h>
#include <libaegis/ael/build_header.h>
#include <libaegis/ael/change/by_state.h>
#include <libaegis/ael/change/inappropriat.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/change.h>
#include <libaegis/change/identifier.h>
#include <libaegis/col.h>
#include <libaegis/option.h>
#include <libaegis/output.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/user.h>


static int
single_bit(int n)
{
    int         result;

    // see if no bits are set
    if (!n)
        return -1;

    //
    // see if more than 1 bit is set
    //  (only works on 2s compliment machines)
    //
    if ((n & -n) != n)
        return -1;

    //
    // will need to extend this for 64bit machines,
    // if ever have >32 states
    //
    result = 0;
    if (n & 0xFFFF0000)
        result += 16;
    if (n & 0xFF00FF00)
        result += 8;
    if (n & 0xF0F0F0F0)
        result += 4;
    if (n & 0xCCCCCCCC)
        result += 2;
    if (n & 0xAAAAAAAA)
        result++;
    return result;
}


static void
output_reaper(void *p)
{
    output::pointer *opp = (output::pointer *)p;
    delete opp;
}


void
list_changes_in_state_mask_by_user(change_identifier &cid, int state_mask,
    string_ty *login)
{
    output::pointer number_col;
    output::pointer state_col;
    output::pointer description_col;
    int             j;
    string_ty       *line1;
    string_ty       *line2;
    symtab_ty       *attr_col_stp = 0;

    if (cid.set())
        list_change_inappropriate();

    //
    // Check that the specified user exists.
    //
    if (login)
        user_ty::create(nstring(login));

    //
    // create the columns
    //
    col::pointer colp = col::open((string_ty *)0);
    line1 =
        str_format("Project \"%s\"", project_name_get(cid.get_pp()).c_str());
    j = single_bit(state_mask);
    if (j >= 0)
    {
        line2 =
            str_format
            (
                "List of Changes %s",
                cstate_state_ename((cstate_state_ty)j)
            );
    }
    else
    {
        j = single_bit(~state_mask);
        if (j >= 0)
        {
            line2 =
                str_format
                (
                    "List of Changes not %s",
                    cstate_state_ename((cstate_state_ty)j)
                );
        }
        else
            line2 = str_from_c("List of Changes");
    }
    colp->title(line1->str_text, line2->str_text);
    str_free(line1);
    str_free(line2);

    int left = 0;
    number_col = colp->create(left, left + CHANGE_WIDTH, "Change\n-------");
    left += CHANGE_WIDTH + 1;

    if (!option_terse_get())
    {
        state_col =
            colp->create(left, left + STATE_WIDTH, "State\n-------");
        left += STATE_WIDTH + 1;

        attr_col_stp = new symtab_ty(5);
        attr_col_stp->set_reap(output_reaper);
        for (j = 0; ; ++j)
        {
            long change_number = 0;
            if (!project_change_nth(cid.get_pp(), j, &change_number))
                break;
            change::pointer cp = change_alloc(cid.get_pp(), change_number);
            change_bind_existing(cp);
            cstate_ty *cstate_data = cp->cstate_get();
            if
            (
                (state_mask & (1 << cstate_data->state))
            &&
                //
                // If no user has been specified, we don't care who the
                // owner of the change is.
                //
                (!login || str_equal(login, cp->developer_name()))
            &&
                cstate_data->attribute
            )
            {
                for (size_t k = 0; k < cstate_data->attribute->length; ++k)
                {
                    attributes_ty *ap = cstate_data->attribute->list[k];
                    if (ael_attribute_listable(ap))
                    {
                        string_ty *lc_name = str_downcase(ap->name);
                        void *p = attr_col_stp->query(lc_name);
                        if (!p)
                        {
                            string_ty *s = ael_build_header(ap->name);
                            output::pointer op =
                                colp->create
                                (
                                    left,
                                    left + ATTR_WIDTH,
                                    s->str_text
                                );
                            str_free(s);
                            attr_col_stp->assign
                            (
                                lc_name,
                                new output::pointer(op)
                            );
                            left += ATTR_WIDTH + 1;
                        }
                        str_free(lc_name);
                    }
                }
            }
            change_free(cp);
        }

        description_col =
            colp->create(left, 0, "Description\n-------------");
    }

    //
    // list the project's changes
    //
    for (j = 0; ; ++j)
    {
        long change_number = 0;
        if (!project_change_nth(cid.get_pp(), j, &change_number))
            break;
        change::pointer cp = change_alloc(cid.get_pp(), change_number);
        change_bind_existing(cp);
        cstate_ty *cstate_data = cp->cstate_get();
        if
        (
            (state_mask & (1 << cstate_data->state))
        &&
            //
            // If no user has been specified, we don't care who the
            // owner of the change is.
            //
            (!login || str_equal(login, cp->developer_name()))
        )
        {
            number_col->fprintf("%4ld", magic_zero_decode(change_number));
            if (state_col)
            {
                state_col->fputs(cstate_state_ename(cstate_data->state));
                if
                (
                    option_verbose_get()
                &&
                    cstate_data->state == cstate_state_being_developed
                )
                {
                    state_col->end_of_line();
                    state_col->fputs(cp->developer_name());
                }
                if
                (
                    option_verbose_get()
                &&
                    cstate_data->state == cstate_state_being_integrated
                )
                {
                    state_col->end_of_line();
                    state_col->fputs(cp->integrator_name());
                }
            }
            if (description_col && cstate_data->brief_description)
            {
                description_col->fputs(cstate_data->brief_description);
            }
            if (attr_col_stp && cstate_data->attribute)
            {
                for (size_t k = 0; k < cstate_data->attribute->length; ++k)
                {
                    attributes_ty *ap = cstate_data->attribute->list[k];
                    if (ap->name && ap->value)
                    {
                        string_ty *lc_name = str_downcase(ap->name);
                        void *vp = attr_col_stp->query(lc_name);
                        if (vp)
                        {
                            output::pointer op = *(output::pointer *)vp;
                            assert(op);
                            op->fputs(ap->value);
                        }
                        str_free(lc_name);
                    }
                }
            }
            colp->eoln();
        }
        change_free(cp);
    }
    if (attr_col_stp)
        delete attr_col_stp;
    trace(("}\n"));
}


void
list_changes_in_state_mask(change_identifier &cid, int state_mask)
{
    list_changes_in_state_mask_by_user(cid, state_mask, 0);
}


// vim: set ts=8 sw=4 et :
