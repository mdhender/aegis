//
// aegis - project change supervisor
// Copyright (C) 1994, 1996, 2002-2008, 2012 Peter Miller
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

#include <common/mem.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/aer/expr/struct.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/aer/value/void.h>

static size_t ntab;
static size_t ntab_max;
static rpt_value_struct **tab;


static void
symtab_push(rpt_value_struct *stp)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    if (ntab >= ntab_max)
    {
        size_t new_ntab_max = ntab_max * 2 + 4;
        rpt_value_struct **new_tab = new rpt_value_struct * [new_ntab_max];
        for (size_t j = 0; j < ntab; ++j)
            new_tab[j] = tab[j];
        delete [] tab;
        tab = new_tab;
        ntab_max = new_ntab_max;
    }
    tab[ntab++] = stp;
}


static void
symtab_pop(void)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(ntab);
    --ntab;
    tab[ntab] = 0;
}


rpt_value_struct *
rpt_expr_struct::symtab_query(void)
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    assert(ntab);
    return tab[ntab - 1];
}


rpt_expr_struct::~rpt_expr_struct()
{
}


rpt_expr_struct::rpt_expr_struct()
{
}


rpt_expr::pointer
rpt_expr_struct::create()
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    return pointer(new rpt_expr_struct());
}


rpt_value::pointer
rpt_expr_struct::evaluate()
    const
{
    trace(("%s\n", __PRETTY_FUNCTION__));
    rpt_value_struct *stp = new rpt_value_struct();
    symtab_push(stp);
    rpt_value::pointer result(stp);
    for (size_t j = 0; ; ++j)
    {
        rpt_expr::pointer ep = nth_child(j);
        if (!ep)
            break;
        rpt_value::pointer vp = ep->evaluate(false, false);
        if (vp->is_an_error())
        {
            symtab_pop();
            return vp;
        }
        assert(dynamic_cast<rpt_value_void *>(vp.get()));
    }
    symtab_pop();
    return result;
}


// vim: set ts=8 sw=4 et :
