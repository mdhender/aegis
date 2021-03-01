/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate news
 */

#include <change/file.h>
#include <error.h> /* for assert */
#include <symtab.h>
#include <trace.h>


fstate_src
change_file_new(cp, file_name)
	change_ty	*cp;
	string_ty	*file_name;
{
	fstate		fstate_data;
	fstate_src	src_data;
	fstate_src	*src_data_p;
	type_ty		*type_p;

	trace(("change_file_new(cp = %08lX)\n{\n"/*}*/, (long)cp));
	fstate_data = change_fstate_get(cp);
	assert(fstate_data->src);
	src_data_p = fstate_src_list_type.list_parse(fstate_data->src, &type_p);
	assert(type_p == &fstate_src_type);
	src_data = fstate_src_type.alloc();
	*src_data_p = src_data;
	src_data->file_name = str_copy(file_name);
	assert(cp->fstate_stp);
	symtab_assign(cp->fstate_stp, file_name, src_data);
	trace(("return %08lX;\n", (long)src_data));
	trace((/*{*/"}\n"));
	return src_data;
}
