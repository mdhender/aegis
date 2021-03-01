//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1997, 1999, 2001-2008 Peter Miller
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

#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/str_list.h>
#include <common/trace.h>
#include <libaegis/aer/list.h>
#include <libaegis/arglex2.h>
#include <libaegis/col.h>
#include <libaegis/dir.h>
#include <libaegis/gonzo.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/rptidx.h>


static void
process(string_ty *dir, const char *nondir, rptidx_where_list_ty *result)
{
    string_ty       *fn;
    int             err;
    rptidx_ty       *data;

    trace(("process((dir = \"%s\", nondir = \"%s\"))\n{\n", dir->str_text,
	nondir));
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
	size_t		j;

	for (j = 0; j < data->where->length; ++j)
	{
	    rptidx_where_ty *in;
	    rptidx_where_ty *out;
	    rptidx_where_ty **out_p;
	    meta_type         *type_p;

	    in = data->where->list[j];
	    if (!in->name || !in->filename)
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
	    trace(("out->name = \"%s\";\n", out->name->str_text));
	}
    }
    rptidx_type.free(data);
    trace(("}\n"));
}


static int
cmp(const void *va, const void *vb)
{
    rptidx_where_ty *a;
    rptidx_where_ty *b;

    a = *(rptidx_where_ty **)va;
    b = *(rptidx_where_ty **)vb;
    return strcasecmp(a->name->str_text, b->name->str_text);
}


void
report_list(void (*usage)(void))
{
    string_list_ty  path;
    rptidx_where_list_ty *result;
    size_t          j;
    output::pointer name_col;
    output::pointer desc_col;
    output::pointer path_col;
    col::pointer colp;

    //
    // read the rest of the command line
    //
    trace(("report_list()\n{\n"));
    arglex();
    while (arglex_token != arglex_token_eoln)
	generic_argument(usage);

    //
    // get all of the report names
    //
    gonzo_report_path(&path);
    result = (rptidx_where_list_ty *)rptidx_where_list_type.alloc();
    for (j = 0; j < path.nstrings; ++j)
    {
	process(path.string[j], "report.index", result);
	process(path.string[j], "report.local", result);
    }
    trace(("result->length = %d;\n", result->length));
    qsort(result->list, result->length, sizeof(result->list[0]), cmp);

    //
    // form the columns for the output
    //
    colp = col::open((string_ty *)0);
    colp->title("List of Reports", (char *)0);
    name_col = colp->create(0, 15, "Name\n------");
    desc_col = colp->create(16, 47, "Description\n-------------");
    path_col = colp->create(48, 0, "Script File\n-------------");

    //
    // name each of the reports
    //
    for (j = 0; j < result->length; ++j)
    {
	rptidx_where_ty *p;

	p = result->list[j];
	name_col->fputs(p->name);
	desc_col->fputs(p->description);
	path_col->fputs(p->filename);
	colp->eoln();
    }
    rptidx_where_list_type.free(result);
    trace(("}\n"));
}
