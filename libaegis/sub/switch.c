/*
 *      aegis - project change supervisor
 *      Copyright (C) 2002, 2003 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate switchs
 */

#include <str.h>
#include <sub.h>
#include <sub/switch.h>
#include <os.h>
#include <trace.h>
#include <wstr_list.h>


wstring_ty *
sub_switch(sub_context_ty *scp, wstring_list_ty *arg)
{
        string_ty       *s;
        wstring_ty      *result;
        long            n;

        trace(("sub_switch()\n{\n"/*}*/));
        if (arg->nitems < 2)
        {
                sub_context_error_set
                (
                        scp,
                        i18n("requires two or more arguments")
                );
                result = 0;
        }
        else
        {
                s = wstr_to_str(arg->item[1]);
                n = atol(s->str_text) + 2;
                str_free(s);
                if (n >= (long)arg->nitems || n < 2)
                {
                        result = wstr_copy(arg->item[arg->nitems - 1]);
                }
                else
                {
                        result = wstr_copy(arg->item[n]);
                }
        }
        trace(("return %8.8lX;\n", (long)result));
        trace((/*{*/"}\n"));
        return result;
}
