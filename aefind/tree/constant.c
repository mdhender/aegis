/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate constant tree nodes
 */

#include <ac/stdio.h>

#include <aer/value/string.h>
#include <str.h>
#include <tree/constant.h>
#include <tree/private.h>


typedef struct tree_constant_ty tree_constant_ty;
struct tree_constant_ty
{
    tree_ty         inherited;
    rpt_value_ty    *value;
};


static void
destructor(tree_ty *tp)
{
    tree_constant_ty *this;

    this = (tree_constant_ty *)tp;
    rpt_value_free(this->value);
}


static void
print(tree_ty *tp)
{
    tree_constant_ty *this;
    rpt_value_ty    *vp;

    this = (tree_constant_ty *)tp;
    vp = rpt_value_stringize(this->value);
    printf("'%s'", rpt_value_string_query(vp)->str_text);
    rpt_value_free(vp);
}


static rpt_value_ty *
evaluate(tree_ty *tp, string_ty *path, struct stat *st)
{
    tree_constant_ty *this;

    this = (tree_constant_ty *)tp;
    return rpt_value_copy(this->value);
}


static int
useful(tree_ty *tp)
{
    return 0;
}


static int
constant(tree_ty *tp)
{
    return 1;
}


static tree_method_ty method =
{
    sizeof(tree_constant_ty),
    "constant",
    destructor,
    print,
    evaluate,
    useful,
    constant,
    0, /* optimize */
};


tree_ty *
tree_constant_new(rpt_value_ty *value)
{
    tree_ty         *tp;
    tree_constant_ty *this;

    tp = tree_new(&method);
    this = (tree_constant_ty *)tp;
    this->value = rpt_value_copy(value);
    return tp;
}
