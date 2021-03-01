//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 2002-2008 Peter Miller
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

#include <common/error.h>
#include <common/mem.h>
#include <common/str.h>
#include <common/trace.h>
#include <common/wstr.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/value.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/sub.h>


rpt_expr::~rpt_expr()
{
    trace(("rpt_expr::~rpt_expr(%08lX)\n", (long)this));
    delete [] child;
}


rpt_expr::rpt_expr() :
    child(0),
    nchild(0),
    nchild_max(0)
{
    trace(("rpt_expr::rpt_expr(%08lX)\n", (long)this));
}


void
rpt_expr::append(const rpt_expr::pointer &ep)
{
    trace(("rpt_expr_append(this = %08lX, ep = %08lX)\n{\n", (long)this,
        (long)ep.get()));
    if (nchild >= nchild_max)
    {
	size_t new_nchild_max = nchild_max * 2 + 4;
	rpt_expr::pointer *new_child = new rpt_expr::pointer [new_nchild_max];
	for (size_t j = 0; j < nchild; ++j)
	    new_child[j] = child[j];
	delete [] child;
        child = new_child;
        nchild_max = new_nchild_max;
    }
    child[nchild++] = ep;

    //
    // update the parent's position
    //
    if (ep->pos)
    {
        if (!pos)
            pos = ep->pos;
        else
            pos = rpt_position::join(pos, ep->pos);
        assert(pos);
        trace(("pos = \"%s\" %ld;\n", pos->get_file_name().c_str(),
            pos->get_line_number()));
    }
    else
    {
        trace(("pos = NULL\n"));
    }
    trace(("}\n"));
}


void
rpt_expr::prepend(const rpt_expr::pointer &ep)
{
    trace(("rpt_expr::prepend(this = %08lX, ep = %08lX)\n{\n", (long)this,
        (long)ep.get()));
    if (nchild >= nchild_max)
    {
	size_t new_nchild_max = nchild_max * 2 + 4;
	rpt_expr::pointer *new_child = new rpt_expr::pointer [new_nchild_max];
	for (size_t j = 0; j < nchild; ++j)
	    new_child[j + 1] = child[j];
	delete [] child;
        child = new_child;
        nchild_max = new_nchild_max;
    }
    else
    {
	for (size_t j = nchild; j > 0; --j)
	    child[j] = child[j - 1];
    }
    nchild++;
    child[0] = ep;

    //
    // update the parent's position
    //
    //assert(ep->pos);
    if (ep->pos)
    {
        if (!pos)
            pos = ep->pos;
        else
            pos = rpt_position::join(pos, ep->pos);
        assert(pos);
    }
    trace(("}\n"));
}


void
rpt_expr::parse_error(const char *fmt)
    const
{
    assert(pos);
    rpt_lex_error(pos, fmt);
}


bool
rpt_expr::lvalue()
    const
{
    return false;
}


rpt_value::pointer
rpt_expr::evaluate(bool resolve_deferred, bool deref)
    const
{
    trace(("rpt_expr::execute(this = %08lX, resdef = %d, deref = %d)\n{\n",
        (long)this, resolve_deferred, deref));
    if (pos)
    {
        trace(("%s\n", pos->representation().c_str()));
    }
    rpt_value::pointer result = evaluate();
    trace(("resolve_deferred = %s\n", (resolve_deferred ? "true" : "false")));
    trace(("result->name() = \"%s\"\n", result->name()));
    if (deref)
    {
        rpt_value_reference *rvrp =
            dynamic_cast<rpt_value_reference *>(result.get());
        if (rvrp)
        {
            trace(("dereference\n"));
            result = rvrp->get();
            assert(!dynamic_cast<rpt_value_reference *>(result.get()));
        }
    }
    if (resolve_deferred)
    {
        result = rpt_value::undefer(result);
    }
    trace(("return %08lX\n", (long)result.get()));
    trace(("}\n"));
    return result;
}


rpt_expr::pointer
rpt_expr::nth_child(size_t n)
    const
{
    if (n >= nchild)
        return pointer();
    return child[n];
}
