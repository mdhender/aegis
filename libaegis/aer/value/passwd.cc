//
// aegis - project change supervisor
// Copyright (C) 1994-1997, 1999, 2002-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/string.h>

#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/passwd.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/sub.h>


rpt_value_passwd::~rpt_value_passwd()
{
}


rpt_value_passwd::rpt_value_passwd()
{
}


rpt_value::pointer
rpt_value_passwd::create()
{
    static rpt_value::pointer vp;
    if (!vp)
        vp = pointer(new rpt_value_passwd());
    return vp;
}


static rpt_value::pointer
pw_to_struct(struct passwd *pw)
{
    trace(("pw_to_struct()\n{\n" ));
    rpt_value_struct *p = new rpt_value_struct();
    rpt_value::pointer result(p);

    trace(("pw_name\n"));
    p->assign("pw_name", rpt_value_string::create(pw->pw_name));

    trace(("pw_uid\n"));
    p->assign("pw_uid", rpt_value_integer::create(pw->pw_uid));

    trace(("pw_gid\n"));
    p->assign("pw_gid", rpt_value_integer::create(pw->pw_gid));

    trace(("pw_gecos\n"));
    nstring full_name(pw->pw_name);
    if (pw->pw_gecos && pw->pw_gecos[0])
        full_name = pw->pw_gecos;
#ifdef HAVE_pw_comment
    else if (pw->pw_comment && pw->pw_comment[0])
        full_name = pw->pw_comment;
#endif

    //
    // Some systems add lots of other stuff to the full name field
    // in the passwd file.  We are only interested in the name.
    //
    const char *comma = strchr(full_name.c_str(), ',');
    if (comma)
        full_name = full_name.substr(0, comma - full_name.c_str());
    rpt_value::pointer fn = rpt_value_string::create(full_name);
    p->assign("pw_gecos", fn);
    p->assign("pw_comment", fn);

    trace(("pw_dir\n"));
    p->assign("pw_dir", rpt_value_string::create(pw->pw_dir));

    trace(("pw_shell\n"));
    p->assign("pw_shell", rpt_value_string::create(pw->pw_shell));

    trace(("return %p;\n", result.get()));
    trace(( "}\n"));
    return result;
}


rpt_value::pointer
rpt_value_passwd::lookup(const rpt_value::pointer &rhs, bool)
    const
{
    trace(("value_passwd::lookup()\n"));
    rpt_value::pointer rhs2 = rpt_value::arithmetic(rhs);
    rpt_value_integer *rhs2ip = dynamic_cast<rpt_value_integer *>(rhs2.get());
    if (rhs2ip)
    {
        trace(("mark\n"));
        int uid = rhs2ip->query();
        struct passwd *pw = getpwuid_cached(uid);
        if (pw)
        {
            return pw_to_struct(pw);
        }

        trace(("mark\n"));
        sub_context_ty sc;
        sc.var_set_long("Number", uid);
        nstring s(sc.subst_intl(i18n("uid $number unknown")));
        return rpt_value_error::create(s);
    }

    trace(("mark\n"));
    rhs2 = rpt_value::stringize(rhs);
    rpt_value_string *rhs2sp = dynamic_cast<rpt_value_string *>(rhs2.get());
    if (rhs2sp)
    {
        trace(("mark\n"));
        struct passwd *pw = getpwnam_cached(rhs2sp->query());
        if (pw)
            return pw_to_struct(pw);

        sub_context_ty sc;
        sc.var_set_string("Name", rhs2sp->query());
        nstring s(sc.subst_intl(i18n("user $name unknown")));
        return rpt_value_error::create(s);
    }

    trace(("mark\n"));
    sub_context_ty sc;
    sc.var_set_charstar("Name1", name());
    sc.var_set_charstar("Name2", rhs->name());
    nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
rpt_value_passwd::keys()
    const
{
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer result(p);
    setpwent();
    for (;;)
    {
        struct passwd *pw = getpwent();
        if (!pw)
            break;

        p->append(rpt_value_string::create(pw->pw_name));
    }
    return result;
}


rpt_value::pointer
rpt_value_passwd::count()
    const
{
    long n = 0;
    setpwent();
    for (;;)
    {
        struct passwd *pw = getpwent();
        if (!pw)
            break;
        ++n;
    }
    return rpt_value_integer::create(n);
}


const char *
rpt_value_passwd::name()
    const
{
    return "passwd";
}


const char *
rpt_value_passwd::type_of()
    const
{
    return "struct";
}


bool
rpt_value_passwd::is_a_struct()
    const
{
    return true;
}


// vim: set ts=8 sw=4 et :
