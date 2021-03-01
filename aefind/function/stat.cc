//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2005 Peter Miller;
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
// MANIFEST: functions to implement stat-based functions
//

#include <config.h> // sets features, so stat.h tells us everything
#include <ac/sys/types.h>
#include <sys/stat.h>

#include <aer/value/error.h>
#include <aer/value/integer.h>
#include <aer/value/string.h>
#include <aer/value/time.h>
#include <cmdline.h>
#include <descend.h>
#include <function/needs.h>
#include <function/stat.h>
#include <str.h>
#include <sub.h>
#include <tree/list.h>
#include <tree/monadic.h>


static rpt_value_ty *
atime_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "atime");
	sub_var_set_long(scp, "Number", 1);
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_time(st->st_atime);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_time(st2.st_atime);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty atime_method =
{
    sizeof(tree_monadic_ty),
    "atime",
    tree_monadic_destructor,
    tree_monadic_print,
    atime_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_atime(tree_list_ty *args)
{
    function_needs_one("atime", args);
    return tree_monadic_new(&atime_method, args->item[0]);
}


static rpt_value_ty *
ctime_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "ctime");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_time(st->st_ctime);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_time(st2.st_ctime);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty ctime_method =
{
    sizeof(tree_monadic_ty),
    "ctime",
    tree_monadic_destructor,
    tree_monadic_print,
    ctime_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_ctime(tree_list_ty *args)
{
    function_needs_one("ctime", args);
    return tree_monadic_new(&ctime_method, args->item[0]);
}


static rpt_value_ty *
mtime_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "mtime");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_time(st->st_mtime);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_time(st2.st_mtime);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty mtime_method =
{
    sizeof(tree_monadic_ty),
    "mtime",
    tree_monadic_destructor,
    tree_monadic_print,
    mtime_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_mtime(tree_list_ty *args)
{
    function_needs_one("mtime", args);
    return tree_monadic_new(&mtime_method, args->item[0]);
}


static rpt_value_ty *
mode_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "mode");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_integer(st->st_mode & 07777);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_integer(st2.st_mode & 07777);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty mode_method =
{
    sizeof(tree_monadic_ty),
    "mode",
    tree_monadic_destructor,
    tree_monadic_print,
    mode_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_mode(tree_list_ty *args)
{
    function_needs_one("mode", args);
    return tree_monadic_new(&mode_method, args->item[0]);
}


static rpt_value_ty *
nlink_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty  *this_thing;
    rpt_value_ty     *vp;
    rpt_value_ty     *svp;
    rpt_value_ty     *result;
    string_ty        *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "nlink");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_integer(st->st_nlink);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_integer(st2.st_nlink);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty nlink_method =
{
    sizeof(tree_monadic_ty),
    "nlink",
    tree_monadic_destructor,
    tree_monadic_print,
    nlink_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_nlink(tree_list_ty *args)
{
    function_needs_one("nlink", args);
    return tree_monadic_new(&nlink_method, args->item[0]);
}


static rpt_value_ty *
ino_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "inode");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_integer(st->st_ino);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_integer(st2.st_ino);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty ino_method =
{
    sizeof(tree_monadic_ty),
    "ino",
    tree_monadic_destructor,
    tree_monadic_print,
    ino_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_ino(tree_list_ty *args)
{
    function_needs_one("inode", args);
    return tree_monadic_new(&ino_method, args->item[0]);
}


static rpt_value_ty *
uid_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "uid");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_unres, s))
	result = rpt_value_integer(st->st_uid);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_integer(st2.st_uid);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty uid_method =
{
    sizeof(tree_monadic_ty),
    "uid",
    tree_monadic_destructor,
    tree_monadic_print,
    uid_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_uid(tree_list_ty *args)
{
    function_needs_one("uid", args);
    return tree_monadic_new(&uid_method, args->item[0]);
}


static rpt_value_ty *
gid_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "gid");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_integer(st->st_gid);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_integer(st2.st_gid);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty gid_method =
{
    sizeof(tree_monadic_ty),
    "gid",
    tree_monadic_destructor,
    tree_monadic_print,
    gid_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_gid(tree_list_ty *args)
{
    function_needs_one("gid", args);
    return tree_monadic_new(&gid_method, args->item[0]);
}


static rpt_value_ty *
size_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "size");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	result = rpt_value_integer(st->st_size);
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	result = rpt_value_integer(st2.st_size);
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);
    return result;
}


static tree_method_ty size_method =
{
    sizeof(tree_monadic_ty),
    "size",
    tree_monadic_destructor,
    tree_monadic_print,
    size_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_size(tree_list_ty *args)
{
    function_needs_one("size", args);
    return tree_monadic_new(&size_method, args->item[0]);
}


static rpt_value_ty *
type_evaluate(tree_ty *tp, string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st)
{
    tree_monadic_ty *this_thing;
    rpt_value_ty    *vp;
    rpt_value_ty    *svp;
    rpt_value_ty    *result;
    string_ty       *s;
    int             n;

    this_thing = (tree_monadic_ty *)tp;
    vp = tree_evaluate(this_thing->arg, path_unres, path, path_res, st);
    if (vp->method->type == rpt_value_type_error)
	return vp;
    svp = rpt_value_stringize(vp);
    if (svp->method->type != rpt_value_type_string)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Function", "type");
	sub_var_set_charstar(scp, "Number", "1");
	sub_var_set_charstar(scp, "Name", vp->method->name);
	rpt_value_free(vp);
	s =
	    subst_intl
	    (
	       	scp,
		i18n("$function: argument $number: string value required (was "
		    "given $name)")
	    );
	sub_context_delete(scp);
	rpt_value_free(svp);
	result = rpt_value_error(0, s);
	str_free(s);
	return result;
    }
    rpt_value_free(vp);

    s = rpt_value_string_query(svp);
    if (str_equal(path_unres, s) || str_equal(path_res, s))
	n = st->st_mode;
    else
    {
	string_ty	*fn;
	string_ty	*resolved_fn;
	struct stat	st2;

	fn = stack_relative(s);
	resolved_fn = stat_stack(fn, &st2);
	n = st2.st_mode;
	str_free(fn);
	str_free(resolved_fn);
    }
    rpt_value_free(svp);

    n &= S_IFMT;
    switch (n)
    {
    case S_IFLNK: s = str_from_c("symbolic_link"); break;
    case S_IFREG: s = str_from_c("file"); break;
    case S_IFDIR: s = str_from_c("directory"); break;
    case S_IFCHR: s = str_from_c("character_special"); break;
    case S_IFBLK: s = str_from_c("block_special"); break;
    case S_IFIFO: s = str_from_c("named_pipe"); break;
    case S_IFSOCK: s = str_from_c("socket"); break;
    default: s = str_format("%d", n >> 12); break;
    }
    result = rpt_value_string(s);
    str_free(s);
    return result;
}


static tree_method_ty type_method =
{
    sizeof(tree_monadic_ty),
    "type",
    tree_monadic_destructor,
    tree_monadic_print,
    type_evaluate,
    tree_monadic_useful,
    tree_monadic_constant,
    tree_monadic_optimize,
};


tree_ty *
function_type(tree_list_ty *args)
{
    function_needs_one("type", args);
    return tree_monadic_new(&type_method, args->item[0]);
}
