//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2002, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to implement the builtin title function
//

#include <aer/expr.h>
#include <aer/func/print.h>
#include <aer/func/title.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <aer/value/void.h>
#include <col.h>
#include <sub.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild <= 2);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    string_ty	*t1;
    string_ty	*t2;
    rpt_value_ty	*vp;

    if (argc >= 1)
    {
	vp = rpt_value_stringize(argv[0]);
	if (vp->method->type != rpt_value_type_string)
	{
	    sub_context_ty  *scp;
	    string_ty	    *s;

	    scp = sub_context_new();
	    rpt_value_free(vp);
	    sub_var_set_charstar(scp, "Function", "title");
	    sub_var_set_long(scp, "Number", 1);
	    sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	    s =
		subst_intl
		(
	    	    scp,
                i18n("$function: argument $number: unable to print $name value")
		);
	    sub_context_delete(scp);
	    vp = rpt_value_error(ep->pos, s);
	    str_free(s);
	    return vp;
	}
	t1 = str_copy(rpt_value_string_query(vp));
	rpt_value_free(vp);
    }
    else
	t1 = str_from_c("");

    if (argc >= 2)
    {
	vp = rpt_value_stringize(argv[1]);
	if (vp->method->type != rpt_value_type_string)
	{
	    sub_context_ty  *scp;
	    string_ty	    *s;

	    scp = sub_context_new();
	    str_free(t1);
	    rpt_value_free(vp);
	    sub_var_set_charstar(scp, "Function", "title");
	    sub_var_set_long(scp, "Number", 2);
	    sub_var_set_charstar(scp, "Name", argv[1]->method->name);
	    s =
		subst_intl
		(
	    	    scp,
                i18n("$function: argument $number: unable to print $name value")
		);
	    sub_context_delete(scp);
	    vp = rpt_value_error(ep->pos, s);
	    str_free(s);
	    return vp;
	}
	t2 = str_copy(rpt_value_string_query(vp));
	rpt_value_free(vp);
    }
    else
	t2 = str_from_c("");

    col_title(rpt_func_print__colp, t1->str_text, t2->str_text);
    str_free(t1);
    str_free(t2);
    return rpt_value_void();
}


rpt_func_ty rpt_func_title =
{
    "title",
    0, // not optimizable
    verify,
    run
};
