//
//      aegis - project change supervisor
//      Copyright (C) 1993-1995, 1998, 2003-2006, 2008, 2012 Peter Miller
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

#include <common/ac/ctype.h>
#include <common/ac/errno.h>

#include <sys/utsname.h>

#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/uname.h>

static string_ty *uname_variant;
static string_ty *uname_node;


static void
uname_init(void)
{
    struct utsname  u;

    if (uname_variant)
        return;
    trace(("uname_init()\n{\n"));
    if (uname(&u) < 0)
    {
        sub_context_ty  *scp;
        int             errno_old;

        errno_old = errno;
        scp = sub_context_new();
        sub_errno_setx(scp, errno_old);
        fatal_intl(scp, i18n("uname: $errno"));
        // NOTREACHED
    }
    uname_variant =
        str_format("%s-%s-%s-%s", u.sysname, u.release, u.version, u.machine);
    trace_string(uname_variant->str_text);
    uname_node = str_from_c(u.nodename);
    trace_string(uname_node->str_text);
    trace(("}\n"));
}


string_ty *
uname_node_get()
{
    trace(("uname_node_get()\n{\n"));
    uname_init();
    trace(("return %p;\n", uname_node));
    trace(("}\n"));
    return uname_node;
}


string_ty *
uname_variant_get()
{
    trace(("uname_default()\n{\n"));
    uname_init();
    trace(("return %p;\n", uname_variant));
    trace(("}\n"));
    return uname_variant;
}


// vim: set ts=8 sw=4 et :
