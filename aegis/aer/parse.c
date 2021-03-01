/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to parse report descriptions
 */

#include <ac/string.h>

#include <aer/expr/name.h>
#include <aer/lex.h>
#include <aer/parse.h>
#include <aer/report.h>
#include <aer/value/list.h>
#include <aer/value/ref.h>
#include <aer/value/string.h>
#include <arglex.h>
#include <col.h>
#include <dir.h>
#include <error.h>
#include <gonzo.h>
#include <os.h>
#include <rptidx.h>
#include <trace.h>


static string_ty *input;
static string_ty *output;
static wlist	arg;


void
report_parse_filename_set(fn)
	string_ty	*fn;
{
	trace(("report_parse_filename_set(fn = \"%s\")\n{\n"/*}*/,
		fn->str_text));
	assert(!input);
	input = str_copy(fn);
	trace((/*{*/"}\n"));
}


void
report_parse_output_set(fn)
	string_ty	*fn;
{
	trace(("report_parse_output_set(fn = \"%s\")\n{\n"/*}*/,
		fn->str_text));
	assert(!output);
	output = str_copy(fn);
	trace((/*{*/"}\n"));
}


void
report_parse_argument_set(a)
	wlist		*a;
{
	trace(("report_parse_argument_set()\n{\n"/*}*/));
	assert(!arg.wl_nwords);
	wl_copy(&arg, a);
	trace((/*{*/"}\n"));
}


static void find_filename_process _((string_ty *, string_ty *, char *,
	rptidx_where_list));

static void
find_filename_process(name, dir, nondir, result)
	string_ty	*name;
	string_ty	*dir;
	char		*nondir;
	rptidx_where_list result;
{
	string_ty	*fn;
	int		err;
	rptidx		data;

	trace(("find_filename_process()\n{\n"/*}*/));
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

	data = rptidx_read_file(fn->str_text);
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
			trace(("arglex_compare(\"%s\", \"%s\")\n",
				in->name->str_text, name->str_text));
			if (!arglex_compare(in->name->str_text, name->str_text))
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
			trace(("out->filename = \"%s\";\n",
				out->filename->str_text));
		}
	}
	rptidx_type.free(data);
	trace((/*{*/"}\n"));
}


static string_ty *find_filename _((string_ty *));

static string_ty *
find_filename(name)
	string_ty	*name;
{
	wlist		path;
	rptidx_where_list result;
	string_ty	*tmp;
	size_t		j;

	/*
	 * find all reports matching the name given
	 */
	trace(("find_filename(name = \"%s\")\n{\n"/*}*/, name->str_text));
	gonzo_report_path(&path);
	result = rptidx_where_list_type.alloc();
	for (j = 0; j < path.wl_nwords; ++j)
	{
		find_filename_process
		(
			name,
			path.wl_word[j],
			"report.index",
			result
		);
		find_filename_process
		(
			name,
			path.wl_word[j],
			"report.local",
			result
		);
	}
	wl_free(&path);

	/*
	 * it is an error if there was no matching report
	 */
	trace(("test if no answer\n"));
	if (!result->length)
	{
		rptidx_where_list_type.free(result);
		fatal("report name \"%S\" is unknown", name);
	}

	/*
	 * it is an error if there is more than one matching report
	 */
	trace(("test if too many answers\n"));
	if (result->length > 1)
	{
		wl_zero(&path);
		for (j = 0; j < result->length; ++j)
			wl_append_unique(&path, result->list[j]->name);
		rptidx_where_list_type.free(result);
		tmp = wl2str(&path, 0, path.wl_nwords - 1, ", ");
		wl_free(&path);
		fatal("report name \"%S\" is ambiguous (%S)", name, tmp);
	}

	/*
	 * is there is exactly one,
	 * we are done
	 */
	trace(("but this bowl was just right\n"));
	trace(("result->list[0]->filename = \"%s\";\n", result->list[0]->filename));
	tmp = str_copy(result->list[0]->filename);
	trace(("rptidx_where_list_type.free(result);\n"));
	rptidx_where_list_type.free(result);
	trace(("return \"%s\";\n", tmp->str_text));
	trace((/*{*/"}\n"));
	return tmp;
}


void
report_parse__init_arg()
{
	rpt_value_ty	*list;
	size_t		j;
	rpt_value_ty	*value;
	string_ty	*name;

	list = rpt_value_list();
	for (j = 1; j < arg.wl_nwords; ++j)
	{
		value = rpt_value_string(arg.wl_word[j]);
		rpt_value_list_append(list, value);
		rpt_value_free(value);
	}
	value = rpt_value_reference(list);
	rpt_value_free(list);

	name = str_from_c("arg");
	rpt_expr_name__init(name, value);
	str_free(name);
	/* do not free value */
}


void
report_run()
{
	/*
	 * find the input file is none has been named
	 */
	trace(("report_run()\n{\n"/*}*/));
	assert(arg.wl_nwords);
	if (!input)
	{
		trace(("find the report script\n"));
		input = find_filename(arg.wl_word[0]);
	}

	/*
	 * parse the report
	 */
	trace(("open the input file\n"));
	os_become_orig();
	rpt_lex_open(input);
	os_become_undo();
	trace(("parse the report\n"));
	report_parse();
	trace(("close the input file\n"));
	rpt_lex_close();

	/*
	 * execute the report
	 */
	trace(("open the output file\n"));
	col_open(output ? output->str_text : (char *)0);
	trace(("interpret the report\n"));
	report_interpret();
	trace(("close the output file\n"));
	col_close();

	/*
	 * release dynamic memory
	 */
	assert(input);
	str_free(input);
	if (output)
		str_free(output);
	wl_free(&arg);
	trace((/*{*/"}\n"));
}
