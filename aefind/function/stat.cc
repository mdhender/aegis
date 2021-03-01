//
//      aegis - project change supervisor
//      Copyright (C) 1997, 1999, 2002-2008 Peter Miller
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

#include <common/config.h> // sets features, so stat.h tells us everything
#include <common/ac/sys/types.h>
#include <common/ac/sys/stat.h>

#include <common/str.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/time.h>
#include <libaegis/sub.h>

#include <aefind/cmdline.h>
#include <aefind/descend.h>
#include <aefind/function/needs.h>
#include <aefind/function/stat.h>
#include <aefind/tree/list.h>
#include <aefind/tree/monadic.h>


tree_atime::~tree_atime()
{
}


tree_atime::tree_atime(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_atime::create(const tree::pointer &a_arg)
{
    return pointer(new tree_atime(a_arg));
}


tree::pointer
tree_atime::create_l(const tree_list &args)
{
    function_needs_one("atime", args);
    return create(args[0]);
}


rpt_value::pointer
tree_atime::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "atime");
        sc.var_set_long("Number", 1);
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_time::create(st->st_atime);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_time::create(st2.st_atime);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_atime::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_atime::name()
    const
{
    return "atime";
}


tree_ctime::~tree_ctime()
{
}


tree_ctime::tree_ctime(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_ctime::create(const tree::pointer &a_arg)
{
    return pointer(new tree_ctime(a_arg));
}


tree::pointer
tree_ctime::create_l(const tree_list &args)
{
    function_needs_one("ctime", args);
    return create(args[0]);
}

rpt_value::pointer
tree_ctime::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "ctime");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_time::create(st->st_ctime);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_time::create(st2.st_ctime);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_ctime::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_ctime::name()
    const
{
    return "ctime";
}


tree_mtime::~tree_mtime()
{
}


tree_mtime::tree_mtime(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_mtime::create(const tree::pointer &a_arg)
{
    return pointer(new tree_mtime(a_arg));
}


tree::pointer
tree_mtime::create_l(const tree_list &args)
{
    function_needs_one("mtime", args);
    return create(args[0]);
}


rpt_value::pointer
tree_mtime::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "mtime");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_time::create(st->st_mtime);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_time::create(st2.st_mtime);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_mtime::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_mtime::name()
    const
{
    return "mtime";
}


tree_mode::~tree_mode()
{
}


tree_mode::tree_mode(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_mode::create(const tree::pointer &a_arg)
{
    return pointer(new tree_mode(a_arg));
}


tree::pointer
tree_mode::create_l(const tree_list &args)
{
    function_needs_one("mode", args);
    return create(args[0]);
}


rpt_value::pointer
tree_mode::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "mode");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_integer::create(st->st_mode & 07777);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_integer::create(st2.st_mode & 07777);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_mode::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_mode::name()
    const
{
    return "mode";
}


tree_nlink::~tree_nlink()
{
}


tree_nlink::tree_nlink(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_nlink::create(const tree::pointer &a_arg)
{
    return pointer(new tree_nlink(a_arg));
}


tree::pointer
tree_nlink::create_l(const tree_list &args)
{
    function_needs_one("nlink", args);
    return create(args[0]);
}


rpt_value::pointer
tree_nlink::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "nlink");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_integer::create(st->st_nlink);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_integer::create(st2.st_nlink);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_nlink::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_nlink::name()
    const
{
    return "nlink";
}


tree_ino::~tree_ino()
{
}


tree_ino::tree_ino(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_ino::create(const tree::pointer &a_arg)
{
    return pointer(new tree_ino(a_arg));
}


tree::pointer
tree_ino::create_l(const tree_list &args)
{
    function_needs_one("ino", args);
    return create(args[0]);
}


rpt_value::pointer
tree_ino::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "inode");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_integer::create(st->st_ino);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_integer::create(st2.st_ino);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_ino::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_ino::name()
    const
{
    return "ino";
}


tree_uid::~tree_uid()
{
}


tree_uid::tree_uid(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_uid::create(const tree::pointer &a_arg)
{
    return pointer(new tree_uid(a_arg));
}


tree::pointer
tree_uid::create_l(const tree_list &args)
{
    function_needs_one("uid", args);
    return create(args[0]);
}


rpt_value::pointer
tree_uid::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "uid");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_unres) == s)
        return rpt_value_integer::create(st->st_uid);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_integer::create(st2.st_uid);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_uid::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_uid::name()
    const
{
    return "uid";
}


tree_gid::~tree_gid()
{
}


tree_gid::tree_gid(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_gid::create(const tree::pointer &a_arg)
{
    return pointer(new tree_gid(a_arg));
}


tree::pointer
tree_gid::create_l(const tree_list &args)
{
    function_needs_one("gid", args);
    return create(args[0]);
}


rpt_value::pointer
tree_gid::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "gid");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_integer::create(st->st_gid);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_integer::create(st2.st_gid);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_gid::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_gid::name()
    const
{
    return "gid";
}


tree_size::~tree_size()
{
}


tree_size::tree_size(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_size::create(const tree::pointer &a_arg)
{
    return pointer(new tree_size(a_arg));
}


tree::pointer
tree_size::create_l(const tree_list &args)
{
    function_needs_one("size", args);
    return create(args[0]);
}


rpt_value::pointer
tree_size::evaluate(string_ty *path_unres, string_ty *path, string_ty *path_res,
    struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty sc;
        sc.var_set_charstar("Function", "size");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    if (nstring(path_unres) == s || nstring(path_res) == s)
        return rpt_value_integer::create(st->st_size);

    string_ty *fn = stack_relative(s.get_ref());
    struct stat st2;
    string_ty *resolved_fn = stat_stack(fn, &st2);
    rpt_value::pointer result = rpt_value_integer::create(st2.st_size);
    str_free(fn);
    str_free(resolved_fn);
    return result;
}


tree::pointer
tree_size::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_size::name()
    const
{
    return "size";
}


tree_type::~tree_type()
{
}


tree_type::tree_type(const tree::pointer &a_arg) :
    tree_monadic(a_arg)
{
}


tree::pointer
tree_type::create(const tree::pointer &a_arg)
{
    return pointer(new tree_type(a_arg));
}


tree::pointer
tree_type::create_l(const tree_list &args)
{
    function_needs_one("type", args);
    return create(args[0]);
}


rpt_value::pointer
tree_type::evaluate(string_ty *path_unres, string_ty *path,
    string_ty *path_res, struct stat *st) const
{
    rpt_value::pointer vp = get_arg()->evaluate(path_unres, path, path_res, st);
    if (vp->is_an_error())
        return vp;
    rpt_value::pointer svp = rpt_value::stringize(vp);
    rpt_value_string *svpp = dynamic_cast<rpt_value_string *>(svp.get());
    if (!svpp)
    {
        sub_context_ty  sc;
        sc.var_set_charstar("Function", "type");
        sc.var_set_charstar("Number", "1");
        sc.var_set_charstar("Name", vp->name());
        nstring s
        (
            sc.subst_intl
            (
                i18n("$function: argument $number: string value required (was "
                    "given $name)")
            )
        );
        return rpt_value_error::create(s);
    }

    nstring s = svpp->query();
    int n = 0;
    if (nstring(path_unres) == s || nstring(path_res) == s)
        n = st->st_mode;
    else
    {
        string_ty *fn = stack_relative(s.get_ref());
        struct stat st2;
        string_ty *resolved_fn = stat_stack(fn, &st2);
        n = st2.st_mode;
        str_free(fn);
        str_free(resolved_fn);
    }

    n &= S_IFMT;
    nstring ts;
    switch (n)
    {
    case S_IFLNK:
        ts = "symbolic_link";
        break;

    case S_IFREG:
        ts = "file";
        break;

    case S_IFDIR:
        ts = "directory";
        break;

    case S_IFCHR:
        ts = "character_special";
        break;

    case S_IFBLK:
        ts = "block_special";
        break;

    case S_IFIFO:
        ts = "named_pipe";
        break;

    case S_IFSOCK:
        ts = "socket";
        break;

    default:
        ts = nstring::format("%d", n >> 12);
        break;
    }
    return rpt_value_string::create(ts);
}


tree::pointer
tree_type::optimize()
    const
{
    tree::pointer tp = create(get_arg()->optimize());
    if (tp->constant())
        tp = tp->optimize_constant();
    return tp;
}


const char *
tree_type::name()
    const
{
    return "type";
}
