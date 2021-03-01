/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 2003 Peter Miller.
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to impliment the builtin columns function
 */

#include <aer/expr.h>
#include <aer/func/columns.h>
#include <aer/func/print.h>
#include <aer/value/integer.h>
#include <aer/value/real.h>
#include <aer/value/string.h>
#include <aer/value/struct.h>
#include <aer/value/void.h>
#include <col.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <str.h>
#include <output.h>


static int
verify(rpt_expr_ty *ep)
{
    return (ep->nchild != 0);
}


static long
extract_integer(rpt_value_ty *vp, int min)
{
    long            result;
    rpt_value_ty    *vp3;

    if (!vp)
	    return -1;
    vp3 = rpt_value_arithmetic(vp);
    switch (vp3->method->type)
    {
    case rpt_value_type_integer:
	result = rpt_value_integer_query(vp3);
	if (result < min)
	    result = -1;
	break;

    case rpt_value_type_real:
	result = (long)rpt_value_real_query(vp3);
	if (result < min)
	    result = -1;
	break;

    default:
	result = -1;
	break;
    }
    rpt_value_free(vp3);
    return result;
}


static string_ty *
extract_string(rpt_value_ty *vp)
{
    string_ty       *result;
    rpt_value_ty    *vp3;

    if (!vp)
	return str_from_c("");
    vp3 = rpt_value_stringize(vp);
    if (vp3->method->type == rpt_value_type_string)
	result = str_copy(rpt_value_string_query(vp3));
    else
	result = str_from_c("");
    rpt_value_free(vp3);
    return result;
}


static rpt_value_ty *
run(rpt_expr_ty *ep, size_t argc, rpt_value_ty **argv)
{
    int             left;
    int             padding;
    int             padding_prev;
    int             right;
    int             rightm;
    int             width;
    rpt_value_ty    *vp;
    size_t          j;
    string_ty       *name;

    /*
     * Nuke the old columns.
     */
    while (rpt_func_print__ncolumns > 0)
    {
	--rpt_func_print__ncolumns;
	j = rpt_func_print__ncolumns;
	output_delete(rpt_func_print__column[j]);
	rpt_func_print__column[j] = 0;
    }

    /*
     * Allocate more space if we need it.
     */
    while (rpt_func_print__ncolumns_max < (int)argc)
    {
	size_t		nbytes;
	size_t		old;

	old = rpt_func_print__ncolumns_max;
	rpt_func_print__ncolumns_max = rpt_func_print__ncolumns_max * 2 + 8;
	nbytes =
	    (
	       	rpt_func_print__ncolumns_max
	    *
	       	sizeof(rpt_func_print__column[0])
	    );
	rpt_func_print__column =
	    (output_ty **)mem_change_size(rpt_func_print__column, nbytes);
	for (j = old; j < (size_t)rpt_func_print__ncolumns_max; ++j)
	    rpt_func_print__column[j] = 0;
    }

    /*
     * Create each of the columns.
     */
    assert(rpt_func_print__colp);
    right = 0;
    padding_prev = 0;
    for (j = 0; j < argc; ++j)
    {
	vp = argv[j];
	if (vp->method->type == rpt_value_type_structure)
	{
	    static string_ty *Left;
	    static string_ty *Name;
	    static string_ty *Padding;
	    static string_ty *Right;
	    static string_ty *Width;
	    rpt_value_ty    *vp2;

	    if (!Left)
		Left = str_from_c("left");
	    vp2 = rpt_value_struct_lookup(vp, Left);
	    left = extract_integer(vp2, 0);

	    if (!Name)
		Name = str_from_c("name");
	    vp2 = rpt_value_struct_lookup(vp, Name);
	    name = extract_string(vp2);

	    if (!Padding)
		Padding = str_from_c("padding");
	    vp2 = rpt_value_struct_lookup(vp, Padding);
	    padding = extract_integer(vp2, 0);

	    if (!Right)
		Right = str_from_c("right");
	    vp2 = rpt_value_struct_lookup(vp, Right);
	    rightm = extract_integer(vp2, 0);

	    if (!Width)
		Width = str_from_c("width");
	    vp2 = rpt_value_struct_lookup(vp, Width);
	    width = extract_integer(vp2, 1);
	}
	else
	{
	    vp = rpt_value_stringize(vp);
	    left = -1;
	    name = extract_string(vp);
	    padding = -1;
	    rightm = -1;
	    width = -1;
	    rpt_value_free(vp);
	}

	/*
	 * padding defaults to 1
	 *	except on the left margin
	 */
	if (padding < 0)
	    padding = (right != 0);

	/*
	 * left defaults to the right of the previous column
	 *
	 * padding applies on the left and on the right
	 */
	if (left < 0)
	{
	    if (rightm > 0 && width > 0 && rightm >= right + width)
	       	left = rightm - width;
	    else if (right)
	    {
	       	if (padding > padding_prev)
		    left = right + padding;
	       	else
		    left = right + padding_prev;
	    }
	    else
	       	left = 0;
	}

	/*
	 * the width defaults to 7
	 * (default padding + default width == 8, so tabs work)
	 */
	if (width < 0)
	{
	    if (rightm > left)
	       	width = rightm - left;
	    else
	       	width = 7;
	}
	if (rightm == 0)
	    right = rightm;
	else
	    right = left + width;

	/*
	 * create the column and set its heading
	 */
	rpt_func_print__column[j] =
	    col_create(rpt_func_print__colp, left, right, name->str_text);
	str_free(name);

	/*
	 * setup for next column
	 */
	padding_prev = padding;
    }
    rpt_func_print__ncolumns = argc;

    /*
     * done
     */
    return rpt_value_void();
}


rpt_func_ty rpt_func_columns =
{
    "columns",
    0, /* not optimizable */
    verify,
    run,
};
