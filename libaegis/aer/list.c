/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1997, 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate report lists
 */

#include <ac/errno.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>

#include <aer/list.h>
#include <arglex2.h>
#include <col.h>
#include <dir.h>
#include <error.h>
#include <gonzo.h>
#include <help.h>
#include <mem.h>
#include <os.h>
#include <output.h>
#include <rptidx.h>
#include <trace.h>
#include <str_list.h>




static void process _((string_ty *, char *, rptidx_where_list));

static void
process(dir, nondir, result)
	string_ty	*dir;
	char		*nondir;
	rptidx_where_list result;
{
	string_ty	*fn;
	int		err;
	rptidx		data;

	trace(("process((dir = \"%s\", nondir = \"%s\"))\n{\n"/*}*/,
		dir->str_text, nondir));
	fn = str_format("%S/%s", dir, nondir);
	os_become_orig();
	err = os_readable(fn);
	if (err)
	{
		os_become_undo();
		str_free(fn);
		trace((/*{*/"}\n"));
		return;
	}
	data = rptidx_read_file(fn);
	os_become_undo();
	str_free(fn);
	if (data->where)
	{
		size_t		j;

		for (j = 0; j < data->where->length; ++j)
		{
			rptidx_where	in;
			rptidx_where	out;
			rptidx_where	*out_p;
			type_ty		*type_p;

			in = data->where->list[j];
			if (!in->name || !in->filename)
				continue;
			assert(result);
			out_p =
				rptidx_where_list_type.list_parse
				(
					result,
					&type_p
				);
			assert(type_p == &rptidx_where_type);
			out = rptidx_where_type.alloc();
			*out_p = out;
			out->name = str_copy(in->name);
			if (in->description)
				out->description = str_copy(in->description);
			else
				out->description = str_copy(in->name);
			if (in->filename->str_text[0] == '/')
				out->filename = str_copy(in->filename);
			else
			{
				out->filename =
					str_format("%S/%S", dir, in->filename);
			}
			trace(("out->name = \"%s\";\n", out->name->str_text));
		}
	}
	rptidx_type.free(data);
	trace((/*{*/"}\n"));
}




static int cmp _((const void *, const void *));

static int
cmp(va, vb)
	const void	*va;
	const void	*vb;
{
	rptidx_where	a;
	rptidx_where	b;

	a = *(rptidx_where *)va;
	b = *(rptidx_where *)vb;
	return strcasecmp(a->name->str_text, b->name->str_text);
}


void
report_list(usage)
	void		(*usage)_((void));
{
	string_list_ty		path;
	rptidx_where_list result;
	size_t		j;
	output_ty	*name_col;
	output_ty	*desc_col;
	output_ty	*path_col;
	col_ty		*colp;

	/*
	 * read the rest of the command line
	 */
	trace(("report_list()\n{\n"/*}*/));
	arglex();
	while (arglex_token != arglex_token_eoln)
		generic_argument(usage);

	/*
	 * get all of the report names
	 */
	gonzo_report_path(&path);
	result = rptidx_where_list_type.alloc();
	for (j = 0; j < path.nstrings; ++j)
	{
		process(path.string[j], "report.index", result);
		process(path.string[j], "report.local", result);
	}
	string_list_destructor(&path);
	trace(("result->length = %ld;\n", result->length));
	qsort(result->list, result->length, sizeof(result->list[0]), cmp);

	/*
	 * form the columns for the output
	 */
	colp = col_open((string_ty *)0);
	col_title(colp, "List of Reports", (char *)0);
	name_col = col_create(colp, 0, 15, "Name\n------");
	desc_col = col_create(colp, 16, 47, "Description\n-------------");
	path_col = col_create(colp, 48, 0, "Script File\n-------------");

	/*
	 * name each of the reports
	 */
	for (j = 0; j < result->length; ++j)
	{
		rptidx_where	p;

		p = result->list[j];
		output_put_str(name_col, p->name);
		output_put_str(desc_col, p->description);
		output_put_str(path_col, p->filename);
		col_eoln(colp);
	}
	rptidx_where_list_type.free(result);
	col_close(colp);
	trace((/*{*/"}\n"));
}
