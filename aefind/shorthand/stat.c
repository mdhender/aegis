/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to implement stat-based shorthands
 */

#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <arglex.h>
#include <function/stat.h>
#include <shorthand/stat.h>
#include <str.h>
#include <sub.h>
#include <tree.h>
#include <tree/arithmetic.h>
#include <tree/constant.h>
#include <tree/list.h>
#include <tree/now.h>
#include <tree/relative.h>
#include <tree/this.h>


tree_ty *
shorthand_atime(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n, int units)
{
    tree_ty	    *tp1;
    tree_ty	    *tp2;
    tree_list_ty    *tlp3;
    tree_ty	    *tp3;
    tree_ty	    *tp4;
    rpt_value_ty    *vp5;
    tree_ty	    *tp5;
    tree_ty	    *tp6;
    rpt_value_ty    *vp7;
    tree_ty	    *tp7;
    tree_ty	    *result;

    /* now */
    tp1 = tree_now_new();

    /* {} */
    tp2 = tree_this_new();

    /* atime({}) */
    tlp3 = tree_list_new();
    tree_list_append(tlp3, tp2);
    tree_delete(tp2);
    tp3 = function_atime(tlp3);
    tree_list_delete(tlp3);

    /* now - atime({}) */
    tp4 = tree_subtract_new(tp1, tp3);
    tree_delete(tp1);
    tree_delete(tp3);

    /* units */
    vp5 = rpt_value_integer(units);
    tp5 = tree_constant_new(vp5);
    rpt_value_free(vp5);

    /* (now - atime({})) / units */
    tp6 = tree_divide_new(tp4, tp5);
    tree_delete(tp4);
    tree_delete(tp5);

    vp7 = rpt_value_integer(n);
    tp7 = tree_constant_new(vp7);
    rpt_value_free(vp7);

    /* (now - atime({})) / units == n */
    result = cmp(tp6, tp7);
    tree_delete(tp6);
    tree_delete(tp7);
    return result;
}


tree_ty *
shorthand_ctime(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n, int units)
{
    tree_ty	    *tp1;
    tree_ty	    *tp2;
    tree_list_ty    *tlp3;
    tree_ty	    *tp3;
    tree_ty	    *tp4;
    rpt_value_ty    *vp5;
    tree_ty	    *tp5;
    tree_ty	    *tp6;
    rpt_value_ty    *vp7;
    tree_ty	    *tp7;
    tree_ty	    *result;

    /* now */
    tp1 = tree_now_new();

    /* {} */
    tp2 = tree_this_new();

    /* ctime({}) */
    tlp3 = tree_list_new();
    tree_list_append(tlp3, tp2);
    tree_delete(tp2);
    tp3 = function_ctime(tlp3);
    tree_list_delete(tlp3);

    /* now - ctime({}) */
    tp4 = tree_subtract_new(tp1, tp3);
    tree_delete(tp1);
    tree_delete(tp3);

    /* units */
    vp5 = rpt_value_integer(units);
    tp5 = tree_constant_new(vp5);
    rpt_value_free(vp5);

    /* (now - ctime({})) / units */
    tp6 = tree_divide_new(tp4, tp5);
    tree_delete(tp4);
    tree_delete(tp5);

    vp7 = rpt_value_integer(n);
    tp7 = tree_constant_new(vp7);
    rpt_value_free(vp7);

    /* (now - ctime({})) / units == n */
    result = cmp(tp6, tp7);
    tree_delete(tp6);
    tree_delete(tp7);
    return result;
}


tree_ty *
shorthand_gid(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* gid({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_gid(tlp2);
    tree_list_delete(tlp2);

    /* n */
    vp3 = rpt_value_integer(n);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* gid({}) == n */
    result = cmp(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}


tree_ty *
shorthand_ino(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* ino({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_ino(tlp2);
    tree_list_delete(tlp2);

    /* n */
    vp3 = rpt_value_integer(n);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* ino({}) == n */
    result = cmp(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}


tree_ty *
shorthand_mode(int n)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* mode({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_mode(tlp2);
    tree_list_delete(tlp2);

    /* n */
    vp3 = rpt_value_integer(n);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* mode({}) == n */
    result = tree_eq_new(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}


tree_ty *
shorthand_mtime(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n, int units)
{
    tree_ty	    *tp1;
    tree_ty	    *tp2;
    tree_list_ty    *tlp3;
    tree_ty	    *tp3;
    tree_ty	    *tp4;
    rpt_value_ty    *vp5;
    tree_ty	    *tp5;
    tree_ty	    *tp6;
    rpt_value_ty    *vp7;
    tree_ty	    *tp7;
    tree_ty	    *result;

    /* now */
    tp1 = tree_now_new();

    /* {} */
    tp2 = tree_this_new();

    /* mtime({}) */
    tlp3 = tree_list_new();
    tree_list_append(tlp3, tp2);
    tree_delete(tp2);
    tp3 = function_mtime(tlp3);
    tree_list_delete(tlp3);

    /* now - mtime({}) */
    tp4 = tree_subtract_new(tp1, tp3);
    tree_delete(tp1);
    tree_delete(tp3);

    /* units */
    vp5 = rpt_value_integer(units);
    tp5 = tree_constant_new(vp5);
    rpt_value_free(vp5);

    /* (now - mtime({})) / units */
    tp6 = tree_divide_new(tp4, tp5);
    tree_delete(tp4);
    tree_delete(tp5);

    /* n */
    vp7 = rpt_value_integer(n);
    tp7 = tree_constant_new(vp7);
    rpt_value_free(vp7);

    /* (now - mtime({})) / units == n */
    result = cmp(tp6, tp7);
    tree_delete(tp6);
    tree_delete(tp7);
    return result;
}


tree_ty *
shorthand_newer(string_ty *filename)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_list_ty    *tlp4;
    tree_ty	    *tp4;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* mtime({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_mtime(tlp2);
    tree_list_delete(tlp2);

    /* filename */
    vp3 = rpt_value_string(filename);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* mtime(filename) */
    tlp4 = tree_list_new();
    tree_list_append(tlp4, tp3);
    tree_delete(tp3);
    tp4 = function_mtime(tlp4);
    tree_list_delete(tlp4);

    /* mtime({}) > mtime(filename) */
    result = tree_gt_new(tp2, tp4);
    tree_delete(tp2);
    tree_delete(tp4);
    return result;
}


tree_ty *
shorthand_nlink(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* nlink({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_nlink(tlp2);
    tree_list_delete(tlp2);

    /* n */
    vp3 = rpt_value_integer(n);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* nlink({}) == n */
    result = cmp(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}


tree_ty *
shorthand_size(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* size({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_size(tlp2);
    tree_list_delete(tlp2);

    /* n */
    vp3 = rpt_value_integer(n);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* size({}) == n */
    result = cmp(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}


tree_ty *
shorthand_uid(tree_ty *(*cmp)(tree_ty *, tree_ty *), int n)
{
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    /* {} */
    tp1 = tree_this_new();

    /* uid({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_uid(tlp2);
    tree_list_delete(tlp2);

    /* n */
    vp3 = rpt_value_integer(n);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* uid({}) == n */
    result = cmp(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}


static string_ty *
type_name_by_pattern(string_ty *abbrev)
{
    typedef struct table_ty table_ty;
    struct table_ty
    {
	char            *pattern;
	char            *name;
    };

    static table_ty table[] =
    {
	{ "Block",		"block_special",	},
	{ "Block_Special",	"block_special",	},
	{ "Character",		"character_special",	},
	{ "Character_Special",	"character_special",	},
	{ "Directory",		"directory",		},
	{ "File",		"file",			},
	{ "Link",		"symbolic_link",	},
	{ "Named_Pipe",		"named_pipe",		},
	{ "First_In_First_Out",	"named_pipe",		},
	{ "Normal",		"file",			},
	{ "Pipe",		"named_pipe",		},
	{ "Plain",		"file",			},
	{ "Socket",		"socket",		},
	{ "Symbolic_Link",	"symbolic_link",	},
    };

    table_ty        *tp;
    sub_context_ty  *scp;

    for (tp = table; tp < ENDOF(table); ++tp)
	if (arglex_compare(tp->pattern, abbrev->str_text))
    	    return str_from_c(tp->name);
    scp = sub_context_new();
    sub_var_set_string(scp, "Name", abbrev);
    fatal_intl(scp, i18n("file type $name unknown"));
    /* NOTREACHED */
    sub_context_delete(scp);
    return str_from_c("unknown");
}


tree_ty *
shorthand_type(string_ty *abbrev)
{
    string_ty	    *name;
    tree_ty	    *tp1;
    tree_list_ty    *tlp2;
    tree_ty	    *tp2;
    rpt_value_ty    *vp3;
    tree_ty	    *tp3;
    tree_ty	    *result;

    name = type_name_by_pattern(abbrev);

    /* {} */
    tp1 = tree_this_new();

    /* type({}) */
    tlp2 = tree_list_new();
    tree_list_append(tlp2, tp1);
    tree_delete(tp1);
    tp2 = function_type(tlp2);
    tree_list_delete(tlp2);

    /* name */
    vp3 = rpt_value_string(name);
    str_free(name);
    tp3 = tree_constant_new(vp3);
    rpt_value_free(vp3);

    /* type({}) == name */
    result = tree_eq_new(tp2, tp3);
    tree_delete(tp2);
    tree_delete(tp3);
    return result;
}
