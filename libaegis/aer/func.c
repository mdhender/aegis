/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1995, 1996, 1997 Peter Miller;
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
 * MANIFEST: functions to manipulate builtin functions
 */

#include <aer/func/change.h>
#include <aer/func/columns.h>
#include <aer/func/count.h>
#include <aer/func/dirname.h>
#include <aer/func/eject.h>
#include <aer/func/getenv.h>
#include <aer/func/gettime.h>
#include <aer/func/keys.h>
#include <aer/func/length.h>
#include <aer/func/need.h>
#include <aer/func/now.h>
#include <aer/func/page_width.h>
#include <aer/func/print.h>
#include <aer/func/project.h>
#include <aer/func/round.h>
#include <aer/func/sort.h>
#include <aer/func/split.h>
#include <aer/func/sprintf.h>
#include <aer/func/strftime.h>
#include <aer/func/substr.h>
#include <aer/func/terse.h>
#include <aer/func/title.h>
#include <aer/func/typeof.h>
#include <aer/func/wrap.h>
#include <aer/value/func.h>
#include <symtab.h>


static rpt_func_ty *table[] =
{
	&rpt_func_basename,
	&rpt_func_ceil,
	&rpt_func_change_number,
	&rpt_func_change_number_set,
	&rpt_func_columns,
	&rpt_func_count,
	&rpt_func_dirname,
	&rpt_func_eject,
	&rpt_func_floor,
	&rpt_func_getenv,
	&rpt_func_gettime,
	&rpt_func_keys,
	&rpt_func_length,
	&rpt_func_mktime,
	&rpt_func_need,
	&rpt_func_now,
	&rpt_func_page_length,
	&rpt_func_page_width,
	&rpt_func_print,
	&rpt_func_project_name,
	&rpt_func_project_name_set,
	&rpt_func_round,
	&rpt_func_sort,
	&rpt_func_split,
	&rpt_func_sprintf,
	&rpt_func_strftime,
	&rpt_func_substr,
	&rpt_func_terse,
	&rpt_func_title,
	&rpt_func_trunc,
	&rpt_func_typeof,
	&rpt_func_working_days,
	&rpt_func_wrap,
	&rpt_func_wrap_html,
};


void
rpt_func_init(stp)
	symtab_ty	*stp;
{
	size_t		j;
	rpt_func_ty	*fp;
	string_ty	*name;
	rpt_value_ty	*data;

	for (j = 0; j < SIZEOF(table); ++j)
	{
		fp = table[j];
		name = str_from_c(fp->name);
		data = rpt_value_func(fp);
		symtab_assign(stp, name, data);
		str_free(name);
	}
}
