//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1995, 1997, 1999, 2001-2008 Peter Miller
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

#include <common/ac/string.h>

#include <common/arglex.h>
#include <common/error.h>
#include <common/trace.h>
#include <libaegis/aer/expr/name.h>
#include <libaegis/aer/func/print.h>
#include <libaegis/aer/lex.h>
#include <libaegis/aer/parse.h>
#include <libaegis/aer/report.h>
#include <libaegis/aer/value/list.h>
#include <libaegis/aer/value/ref.h>
#include <libaegis/aer/value/string.h>
#include <libaegis/col.h>
#include <libaegis/dir.h>
#include <libaegis/gonzo.h>
#include <libaegis/os.h>
#include <libaegis/rptidx.h>
#include <libaegis/sub.h>


static string_ty *input;
static string_ty *output;
static string_list_ty arg;


void
report_parse_filename_set(string_ty *fn)
{
    trace(("report_parse_filename_set(fn = \"%s\")\n{\n", fn->str_text));
    assert(!input);
    input = str_copy(fn);
    trace(("}\n"));
}


void
report_parse_output_set(string_ty *fn)
{
    trace(("report_parse_output_set(fn = \"%s\")\n{\n", fn->str_text));
    assert(!output);
    output = str_copy(fn);
    trace(("}\n"));
}


void
report_parse_argument_set(string_list_ty *a)
{
    trace(("report_parse_argument_set()\n{\n"));
    assert(!arg.nstrings);
    arg = *a;
    trace(("}\n"));
}


static void
find_filename_process(string_ty *name, string_ty *dir, const char *nondir,
    rptidx_where_list_ty *result)
{
    string_ty       *fn;
    int             err;
    rptidx_ty       *data;

    trace(("find_filename_process()\n{\n"));
    fn = str_format("%s/%s", dir->str_text, nondir);
    os_become_orig();
    err = os_readable(fn);
    if (err)
    {
	os_become_undo();
	str_free(fn);
	trace(("}\n"));
	return;
    }

    data = rptidx_read_file(fn);
    os_become_undo();
    str_free(fn);
    if (data->where)
    {
	size_t		j, k;

	for (j = 0; j < data->where->length; ++j)
	{
	    rptidx_where_ty *in;
	    rptidx_where_ty *out;
	    rptidx_where_ty **out_p;
	    meta_type         *type_p;
	    int             have_it_already;

	    in = data->where->list[j];
	    if (!in->name || !in->filename)
		continue;
	    trace(("arglex_compare(\"%s\", \"%s\")\n",
		in->name->str_text, name->str_text));
	    if (!arglex_compare(in->name->str_text, name->str_text, 0))
		continue;

	    //
	    // If we already have this one, ignore it.
	    // (Duplicate entries in AEGIS_PATH will cause
	    // in exact duplicates in the result list.)
	    //
	    have_it_already = 0;
	    for (k = 0; k < result->length; ++k)
	    {
		out = result->list[k];
		if (str_equal(in->name, out->name))
		{
	    	    have_it_already = 1;
	    	    break;
		}
	    }
	    if (have_it_already)
		continue;

	    assert(result);
	    out_p =
		(rptidx_where_ty **)
                rptidx_where_list_type.list_parse(result, &type_p);
	    assert(type_p == &rptidx_where_type);
	    out = (rptidx_where_ty *)rptidx_where_type.alloc();
	    *out_p = out;
	    out->name = str_copy(in->name);
	    if (in->description)
		out->description = str_copy(in->description);
	    else
		out->description = str_copy(in->name);
	    if (in->filename->str_text[0] == '/')
		out->filename = str_copy(in->filename);
	    else
		out->filename = os_path_join(dir, in->filename);
	    trace(("out->filename = \"%s\";\n", out->filename->str_text));
	}
    }
    rptidx_type.free(data);
    trace(("}\n"));
}


static string_ty *
find_filename(string_ty *name)
{
    rptidx_where_list_ty *result;
    string_ty       *tmp;
    size_t          j;

    //
    // find all reports matching the name given
    //
    trace(("find_filename(name = \"%s\")\n{\n", name->str_text));
    string_list_ty path;
    gonzo_report_path(&path);
    result = (rptidx_where_list_ty *)rptidx_where_list_type.alloc();
    for (j = 0; j < path.nstrings; ++j)
    {
	find_filename_process(name, path.string[j], "report.index", result);
	find_filename_process(name, path.string[j], "report.local", result);
    }

    //
    // it is an error if there was no matching report
    //
    trace(("test if no answer\n"));
    if (!result->length)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	rptidx_where_list_type.free(result);
	sub_var_set_string(scp, "Name", name);
	fatal_intl(scp, i18n("no report $name"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // it is an error if there is more than one matching report
    //
    trace(("test if too many answers\n"));
    if (result->length > 1)
    {
	string_list_ty path2;
	for (j = 0; j < result->length; ++j)
	    path2.push_back_unique(result->list[j]->name);
	rptidx_where_list_type.free(result);
	tmp = path2.unsplit(", ");
	sub_context_ty *scp = sub_context_new();
	sub_var_set_string(scp, "Name", name);
	sub_var_set_string(scp, "Name_List", tmp);
	sub_var_optional(scp, "Name_List");
	fatal_intl(scp, i18n("report $name ambig"));
	// NOTREACHED
	sub_context_delete(scp);
    }

    //
    // is there is exactly one,
    // we are done
    //
    trace(("but this bowl was just right\n"));
    trace(("result->list[0]->filename = \"%s\";\n",
	result->list[0]->filename->str_text));
    tmp = str_copy(result->list[0]->filename);
    trace(("rptidx_where_list_type.free(result);\n"));
    rptidx_where_list_type.free(result);
    trace(("return \"%s\";\n", tmp->str_text));
    trace(("}\n"));
    return tmp;
}


void
report_parse__init_arg()
{
    rpt_value_list *p = new rpt_value_list();
    rpt_value::pointer list(p);
    for (size_t j = 1; j < arg.nstrings; ++j)
    {
	p->append(rpt_value_string::create(nstring(arg.string[j])));
    }
    rpt_value::pointer value = rpt_value_reference::create(list);

    rpt_expr_name__init("arg", value);
}


void
report_run()
{
    //
    // find the input file is none has been named
    //
    trace(("report_run()\n{\n"));
    assert(arg.nstrings);
    if (!input)
    {
	trace(("find the report script\n"));
	input = find_filename(arg.string[0]);
    }

    //
    // parse the report
    //
    trace(("open the input file\n"));
    os_become_orig();
    rpt_lex_open(input);
    trace(("parse the report\n"));
    aer_report_parse();
    trace(("close the input file\n"));
    rpt_lex_close();
    os_become_undo();

    //
    // execute the report
    //
    trace(("open the output file\n"));
    assert(!rpt_func_print__colp);
    rpt_func_print__colp = col::open(output);
    trace(("interpret the report\n"));
    report_interpret();
    trace(("close the output file\n"));
    rpt_func_print__colp.reset();

    //
    // release dynamic memory
    //
    assert(input);
    str_free(input);
    if (output)
	str_free(output);
    arg.clear();
    trace(("}\n"));
}
