//
//	aegis - project change supervisor
//	Copyright (C) 1997, 1999, 2002-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <aefind/lex.h>
#include <aefind/function/needs.h>
#include <libaegis/sub.h>
#include <aefind/tree.h>
#include <aefind/tree/list.h>
#include <aefind/tree/this.h>


void
function_needs_one(const char *name, const tree_list &args)
{
    if (args.size() == 1)
	return;

    sub_context_ty sc;
    sc.var_set_charstar("Name", name);
    cmdline_lex_error(&sc, i18n("function $name requires one argument"));
}


void
function_needs_two(const char *name, const tree_list &args)
{
    if (args.size() == 2)
	return;

    sub_context_ty  *scp;
    scp = sub_context_new();
    sub_var_set_charstar(scp, "Name", name);
    cmdline_lex_error(scp, i18n("function $name requires two arguments"));
    sub_context_delete(scp);
}
