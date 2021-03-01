//
//	aegis - project change supervisor
//	Copyright (C) 1996, 1999, 2002, 2004, 2005 Peter Miller;
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
// MANIFEST: functions to manipulate strftime
//

#include <common/ac/time.h>

#include <libaegis/aer/expr.h>
#include <libaegis/aer/func/strftime.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <common/str.h>
#include <libaegis/sub.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild == 2);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    string_ty	    *t1;
    time_t	    t2;
    rpt_value_ty    *vp1;
    rpt_value_ty    *vp2;
    rpt_value_ty    *result;
    int		    nbytes;
    string_ty	*    s;
    char		buffer[2000];

    if (argc >= 1)
    {
	vp1 = rpt_value_stringize(argv[0]);
	if (vp1->method->type != rpt_value_type_string)
	{
	    sub_context_ty	*scp;

	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Function", "strftime");
	    sub_var_set_long(scp, "Number", 1);
	    sub_var_set_charstar(scp, "Name", argv[0]->method->name);
	    rpt_value_free(vp1);
	    s =
		subst_intl
		(
	    	    scp,
    i18n("$function: argument $number: string value required (was given $name)")
		);
	    sub_context_delete(scp);
	    result = rpt_value_error(ep->pos, s);
	    str_free(s);
	    return result;
	}
	t1 = str_copy(rpt_value_string_query(vp1));
	rpt_value_free(vp1);
    }
    else
	t1 = str_from_c("%C");

    if (argc >= 2)
    {
	vp2 = rpt_value_integerize(argv[1]);
	if (vp2->method->type != rpt_value_type_integer)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    rpt_value_free(vp2);
	    sub_var_set_charstar(scp, "Function", "strftime");
	    sub_var_set_long(scp, "Number", 2);
	    sub_var_set_charstar(scp, "Name", argv[1]->method->name);
	    s =
		subst_intl
		(
		    scp,
      i18n("$function: argument $number: time value required (was given $name)")
		);
	    sub_context_delete(scp);
	    result = rpt_value_error(ep->pos, s);
	    str_free(s);
	    return result;
	}
	t2 = rpt_value_integer_query(vp2);
	rpt_value_free(vp2);
    }
    else
	t2 = 0;

    nbytes = strftime(buffer, sizeof(buffer), t1->str_text, localtime(&t2));
    if (nbytes <= 0)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "strftime");
	s = subst_intl(scp, i18n("$function: result too long"));
	sub_context_delete(scp);
	result = rpt_value_error(ep->pos, s);
	str_free(s);
    }
    else
    {
	s = str_from_c(buffer);
	result = rpt_value_string(s);
	str_free(s);
    }
    str_free(t1);
    return result;
}


rpt_func_ty rpt_func_strftime =
{
    "strftime",
    1, // optimizable
    verify,
    run
};
