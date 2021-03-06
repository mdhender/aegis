//
//      aegis - project change supervisor
//      Copyright (C) 2001, 2003-2006, 2008, 2009, 2012 Peter Miller
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

#include <libaegis/input.h>
#include <libaegis/sub.h>


void
input_ty::fatal_error(const char *msg)
{
    sub_context_ty sc;
    string_ty *s = sc.subst_intl(msg);
    sc.var_set_string("File_Name", name());
    sc.var_set_string("MeSsaGe", s);
    sc.fatal_intl(i18n("$filename: $message"));
    // NOTREACHED
}


void
input_ty::error(const char *msg)
{
    sub_context_ty sc;
    string_ty *s = sc.subst_intl(msg);
    sc.var_set_string("File_Name", name());
    sc.var_set_string("MeSsaGe", s);
    sc.error_intl(i18n("$filename: $message"));
}


// vim: set ts=8 sw=4 et :
