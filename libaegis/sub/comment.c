/*
 *	aegis - project change supervisor
 *	Copyright (C) 1997 Peter Miller;
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
 * MANIFEST: functions to impliment comment substitutions
 */

#include <sub/comment.h>
#include <trace.h>
#include <wstr_list.h>


/*
 * NAME
 *	sub_comment - substitute a comment
 *
 * SYNOPSIS
 *	string_ty *sub_comment(wlist *arg);
 *
 * DESCRIPTION
 *	The sub_comment function implements the comment substitution.
 *	The comment substitution is replaced by exactly nothing.
 *
 * ARGUMENTS
 *	arg	- list of arguments, including the function name as [0]
 *
 * RETURNS
 *	a pointer to a string in dynamic memory;
 *	or NULL on error, setting suberr appropriately.
 */

wstring_ty *
sub_comment(scp, arg)
	struct sub_context_ty *scp;
	wstring_list_ty	*arg;
{
	wstring_ty	*result;

	trace(("sub_comment()\n{\n"/*}*/));
	result = wstr_from_c("");
	trace((/*{*/"}\n"));
	return result;
}
