//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 1999, 2001-2004 Peter Miller;
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
// MANIFEST: functions to implement the builtin print function
//

#include <aer/expr.h>
#include <aer/func/print.h>
#include <aer/value/void.h>
#include <aer/value/error.h>
#include <aer/value/string.h>
#include <col.h>
#include <error.h>
#include <mem.h>
#include <os.h>
#include <output.h>
#include <sub.h>


int		rpt_func_print__ncolumns;
int		rpt_func_print__ncolumns_max;
output_ty	**rpt_func_print__column;
col_ty		*rpt_func_print__colp;


static int
verify(rpt_expr_ty *ep)
{
    if (!rpt_func_print__ncolumns)
	return 0;

    //
    // you may not print with more arguments
    // than columns defined
    //
    return ((int)ep->nchild <= rpt_func_print__ncolumns);
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    size_t	    j;
    size_t          k;
    rpt_value_ty    *vp;
    static size_t   argc2;
    static rpt_value_ty **argv2;
    string_ty	    *s;

    if (argc > (size_t)rpt_func_print__ncolumns)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "print");
	sub_var_set_long(scp, "Number1", (long)argc);
	sub_var_set_long(scp, "Number2", (long)rpt_func_print__ncolumns);
	s =
	    subst_intl
	    (
		scp,
      i18n("$function: too many arguments ($number1 given, only $number2 used)")
	    );
	sub_context_delete(scp);
	vp = rpt_value_error(ep->pos, s);
	str_free(s);
	return vp;
    }

    //
    // allocate more space for argv2 if necessary
    //
    if (argc > argc2)
    {
	argc2 = argc;
	argv2 =
            (rpt_value_ty **)mem_change_size(argv2, argc2 * sizeof(argv2[0]));
    }

    //
    // turn each argument into a string
    //
    for (j = 0; j < argc; ++j)
    {
	sub_context_ty	*scp;

	vp = rpt_value_stringize(argv[j]);
	if (vp->method->type == rpt_value_type_string)
	{
	    argv2[j] = vp;
	    continue;
	}

	//
	// If it refuses to become a string,
	// free up the converted values...
	//
	rpt_value_free(vp);
	for (k = 0; k < j; ++k)
	    rpt_value_free(argv2[k]);
	mem_free(argv2);

	//
	// ...and complain bitterly
	//
	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "print");
	sub_var_set_long(scp, "Number", (long)j + 1);
	sub_var_set_charstar(scp, "Name", argv[j]->method->name);
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

    //
    // now that we know they are all stringizable, print the line out
    //
    for (j = 0; j < argc; ++j)
    {
	vp = rpt_value_stringize(argv2[j]);
	assert(vp->method->type==rpt_value_type_string);
	if (rpt_func_print__column[j])
	{
	    output_put_str
	    (
		rpt_func_print__column[j],
		rpt_value_string_query(vp)
	    );
	}
    }

    //
    // Emit the line.
    //
    // The os_become bracketing is because we would write to the
    // file at this point, and some operatings systems will barf if
    // we have the wrong uid.
    //
    os_become_orig();
    col_eoln(rpt_func_print__colp);
    os_become_undo();

    //
    // free all the stringized values
    // (but keep argv2, we will need it again)
    //
    for (j = 0; j < argc; ++j)
	rpt_value_free(argv2[j]);
    return rpt_value_void();
}


rpt_func_ty rpt_func_print =
{
    "print",
    0, // not optimizable
    verify,
    run
};
