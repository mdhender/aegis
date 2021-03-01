/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997, 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate function needs
 */

#include <lex.h>
#include <function/needs.h>
#include <sub.h>
#include <tree.h>
#include <tree/list.h>
#include <tree/this.h>


void
function_needs_one(name, args)
	char		*name;
	tree_list_ty	*args;
{
	sub_context_ty	*scp;

	if (args->length == 1)
		return;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", name);
	cmdline_lex_error
	(
		scp,
		i18n("function $name requires one argument")
	);
	sub_context_delete(scp);
	
	if (args->length < 1)
	{
		tree_ty		*tp;

		tp = tree_this_new();
		tree_list_append(args, tp);
		tree_delete(tp);
	}
}


void
function_needs_two(name, args)
	char		*name;
	tree_list_ty	*args;
{
	sub_context_ty	*scp;

	if (args->length == 2)
		return;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", name);
	cmdline_lex_error
	(
		scp,
		i18n("function $name requires two arguments")
	);
	sub_context_delete(scp);
	
	while (args->length < 2)
	{
		tree_ty		*tp;

		tp = tree_this_new();
		tree_list_append(args, tp);
		tree_delete(tp);
	}
}
