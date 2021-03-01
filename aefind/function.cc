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

#include <aefind/function.h>
#include <aefind/function/basename.h>
#include <aefind/function/execute.h>
#include <aefind/function/print.h>
#include <aefind/function/stat.h>
#include <aefind/lex.h>
#include <libaegis/sub.h>
#include <common/symtab.h>
#include <aefind/tree.h>
#include <aefind/tree/list.h>
#include <aefind/tree/this.h>


struct table_ty
{
    const char *name;
    tree::pointer (*func)(const tree_list &);
};


static table_ty table[] =
{
    { "access_time", &tree_atime::create_l, },
    { "atime", &tree_atime::create_l, },
    { "basename", &tree_basename::create_l, },
    { "change_time", &tree_ctime::create_l, },
    { "ctime", &tree_ctime::create_l, },
    { "execute", &tree_execute::create_l, },
    { "gid", &tree_gid::create_l, },
    { "group_id", &tree_gid::create_l, },
    { "ino", &tree_ino::create_l, },
    { "inode", &tree_ino::create_l, },
    { "mode", &tree_mode::create_l, },
    { "modify_time", &tree_mtime::create_l, },
    { "mtime", &tree_mtime::create_l, },
    { "nlink", &tree_nlink::create_l, },
    { "print", &tree_print::create_l, },
    { "size", &tree_size::create_l, },
    { "st_atime", &tree_atime::create_l, },
    { "st_ctime", &tree_ctime::create_l, },
    { "st_gid", &tree_gid::create_l, },
    { "st_ino", &tree_ino::create_l, },
    { "st_mode", &tree_mode::create_l, },
    { "st_mtime", &tree_mtime::create_l, },
    { "st_nlink", &tree_nlink::create_l, },
    { "st_size", &tree_size::create_l, },
    { "st_uid", &tree_uid::create_l, },
    { "type", &tree_type::create_l, },
    { "uid", &tree_uid::create_l, },
    { "user_id", &tree_uid::create_l, },
};


tree::pointer
function_indirection(string_ty *name, const tree_list &args)
{
    static symtab_ty *stp;
    if (!stp)
    {
	stp = symtab_alloc(SIZEOF(table));
	for (table_ty *tp = table; tp < ENDOF(table); ++tp)
	{
	    string_ty *s = str_from_c(tp->name);
	    symtab_assign(stp, s, tp);
	    str_free(s);
	}
    }

    table_ty *tp = (table_ty *)symtab_query(stp, name);
    if (!tp)
    {
	string_ty *guess = symtab_query_fuzzy(stp, name);
	if (!guess)
	{
	    sub_context_ty sc;
	    sc.var_set_string("Name", name);
	    cmdline_lex_error(&sc, i18n("the name \"$name\" is undefined"));
	}
	else
	{
	    sub_context_ty sc;
	    sc.var_set_string("Name", name);
	    sc.var_set_string("Guess", guess);
	    cmdline_lex_error(&sc, i18n("no \"$name\", guessing \"$guess\""));
	}
	return tree_this::create();
    }
    return tp->func(args);
}
