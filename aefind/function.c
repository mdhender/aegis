/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate function tree nodes
 */

#include <function.h>
#include <function/basename.h>
#include <function/execute.h>
#include <function/print.h>
#include <function/stat.h>
#include <lex.h>
#include <sub.h>
#include <symtab.h>
#include <tree.h>
#include <tree/list.h>
#include <tree/this.h>


typedef struct table_ty table_ty;
struct table_ty
{
    const char      *name;
    tree_ty         *(*func)(tree_list_ty *);
};


static table_ty table[] =
{
    { "access_time", function_atime, },
    { "atime", function_atime, },
    { "basename", function_basename, },
    { "change_time", function_ctime, },
    { "ctime", function_ctime, },
    { "execute", function_execute, },
    { "gid", function_gid, },
    { "group_id", function_gid, },
    { "ino", function_ino, },
    { "inode", function_ino, },
    { "mode", function_mode, },
    { "modify_time", function_mtime, },
    { "mtime", function_mtime, },
    { "nlink", function_nlink, },
    { "print", function_print, },
    { "size", function_size, },
    { "st_atime", function_atime, },
    { "st_ctime", function_ctime, },
    { "st_gid", function_gid, },
    { "st_ino", function_ino, },
    { "st_mode", function_mode, },
    { "st_mtime", function_mtime, },
    { "st_nlink", function_nlink, },
    { "st_size", function_size, },
    { "st_uid", function_uid, },
    { "type", function_type, },
    { "uid", function_uid, },
    { "user_id", function_uid, },
};


tree_ty *
function_indirection(string_ty *name, tree_list_ty *args)
{
    table_ty        *tp;
    string_ty       *s;
    static symtab_ty *stp;

    if (!stp)
    {
	stp = symtab_alloc(SIZEOF(table));
	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    s = str_from_c(tp->name);
	    symtab_assign(stp, s, tp);
	    str_free(s);
	}
    }

    tp = (table_ty *)symtab_query(stp, name);
    if (!tp)
    {
	string_ty	*guess;

	guess = symtab_query_fuzzy(stp, name);
	if (!guess)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    cmdline_lex_error(scp, i18n("the name \"$name\" is undefined"));
	    sub_context_delete(scp);
	}
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    sub_var_set_string(scp, "Guess", guess);
	    cmdline_lex_error(scp, i18n("no \"$name\", guessing \"$guess\""));
	    sub_context_delete(scp);
	}
	return tree_this_new();
    }
    return tp->func(args);
}
