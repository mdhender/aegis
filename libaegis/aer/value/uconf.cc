//
//      aegis - project change supervisor
//      Copyright (C) 2000-2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/trace.h>
#include <libaegis/aer/value/error.h>
#include <libaegis/aer/value/integer.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/aer/value/struct.h>
#include <libaegis/aer/value/uconf.h>
#include <libaegis/getpw_cache.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


rpt_value_uconf::~rpt_value_uconf()
{
}


rpt_value_uconf::rpt_value_uconf()
{
}


rpt_value::pointer
rpt_value_uconf::create()
{
    static rpt_value::pointer vp;
    if (!vp)
        vp = pointer(new rpt_value_uconf());
    return vp;
}


static rpt_value::pointer
build_result(struct passwd *pw)
{
    trace(("build_struct()\n"));

    //
    // Use the same method as other portions of Aegis to establish
    // the user's attributes.  It is *not* as simple as reading the
    // $HOME/.aegisrc file.
    //
    // The side-effect of the aparrently-useless up->get_email_address()
    // method call is to establish the tmp->email_address field
    // correctly.
    //
    user_ty::pointer up = user_ty::create(pw->pw_uid);
    up->get_email_address();
    uconf_ty *tmp = up->uconf_get();

    //
    // Convert the data
    //
    rpt_value::pointer result = uconf_type.convert(&tmp);
    rpt_value_struct *rvsp = dynamic_cast<rpt_value_struct *>(result.get());
    assert(rvsp);

    //
    // Insert the user name into the result.
    //
    trace(("name\n"));
    rvsp->assign("name", rpt_value_string::create(pw->pw_name));

    //
    // All done.
    //
    trace(("return %p;\n", result.get()));
    trace(("}\n"));
    return result;
}


rpt_value::pointer
rpt_value_uconf::lookup(const rpt_value::pointer &rhs, bool)
    const
{
    trace(("value_uconf::lookup()\n"));
    rpt_value::pointer rhs2 = rpt_value::arithmetic(rhs);
    rpt_value_integer *rhs2ip = dynamic_cast<rpt_value_integer *>(rhs2.get());
    if (rhs2ip)
    {
        int uid = rhs2ip->query();
        struct passwd *pw = getpwuid_cached(uid);
        if (pw)
            return build_result(pw);

        sub_context_ty sc;
        sc.var_set_long("Number", uid);
        nstring s(sc.subst_intl(i18n("uid $number unknown")));
        return rpt_value_error::create(s);
    }

    rhs2 = rpt_value::stringize(rhs);
    rpt_value_string *rhs2sp = dynamic_cast<rpt_value_string *>(rhs2.get());
    if (rhs2sp)
    {
        struct passwd *pw = getpwnam_cached(rhs2sp->query());
        if (pw)
            return build_result(pw);

        sub_context_ty sc;
        sc.var_set_string("Name", rhs2sp->query());
        nstring s(sc.subst_intl(i18n("user $name unknown")));
        return rpt_value_error::create(s);
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name1", name());
    sc.var_set_charstar("Name2", rhs->name());
    nstring s(sc.subst_intl(i18n("illegal lookup ($name1[$name2])")));
    return rpt_value_error::create(s);
}


rpt_value::pointer
rpt_value_uconf::keys()
    const
{
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer result(p);
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
rpt_value_uconf::count()
    const
{
    return rpt_value_integer::create(0);
}


const char *
rpt_value_uconf::type_of()
    const
{
    return "struct";
}


const char *
rpt_value_uconf::name()
    const
{
    return "user";
}


bool
rpt_value_uconf::is_a_struct()
    const
{
    return true;
}


// vim: set ts=8 sw=4 et :
