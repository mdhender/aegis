//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2006 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate prettys
//

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/col/pretty.h>
#include <libaegis/option.h>
#include <libaegis/output/to_wide.h>
#include <libaegis/wide_output/column.h>
#include <libaegis/wide_output/expand.h>
#include <libaegis/wide_output/header.h>
#include <libaegis/wide_output/head.h>
#include <libaegis/wide_output/unexpand.h>
#include <libaegis/wide_output/wrap.h>


col_pretty::~col_pretty()
{
    trace(("col_pretty::destructor(this = %08lX)\n{\n", (long)this));
    trace(("ncolumns = %d\n", (int)ncolumns));
    for (size_t j = 0; j < ncolumns; ++j)
    {
	// The delcb() will do all the work.
	column_ty *cp = &column[j];
	trace(("cp = %08lX\n", (long)cp));
	if (cp->content_filter)
	{
	    delete cp->content_filter;
	    cp->content_filter = 0;
	}
    }
    trace(("column = %08lX\n", (long)column));
    delete [] column;
    trace(("deeper = %08lX\n", (long)deeper));
    wide_output_delete(deeper);
    deeper = 0;
    trace(("}\n"));
}


static wide_output_ty *
mangle(wide_output_ty *arg, bool doc)
{
    arg = wide_output_unexpand_open(arg, doc, -1);
    if (option_page_headers_get())
	arg = wide_output_header_open(arg, 1);
    return arg;
}


col_pretty::col_pretty(wide_output_ty *arg1, bool arg2) :
    deeper(mangle(arg1, arg2)),
    need_to_emit_headers(false),
    ncolumns(0),
    ncolumns_max(0),
    column(0)
{
    trace(("col_pretty::col_pretty(this = %08lX, deeper = %08lX)\n{\n",
	(long)this, (long)deeper));
    trace(("}\n"));
}


void
col_pretty::emit(size_t argc, emit_ty *argv, int minlines,
    bool this_is_the_header)
{
    //
    // Figure out how many lines of output we will produce, and ask
    // for a page eject if that many lines aren't left on the page.
    //
    trace(("col_pretty::emit(this = %08lX)\n{\n", (long)this));
    int lines_needed = 0;
    for (int line = 0; ; ++line)
    {
	bool there_was_something_on_this_line = (line < minlines);
	int ocol = 0;
	for (int j = 0; j < (int)argc; ++j)
	{
	    emit_ty *ep = &argv[j];
	    if (!ep->content)
		continue;
	    trace(("ep = %08lX;\n", (long)ep));
	    column_row_ty *crp = wide_output_column_get(ep->content, line);
	    if (!crp)
		continue;
	    trace(("crp = %08lX;\n", (long)crp));
	    ++there_was_something_on_this_line;
	    if (!crp->length)
		continue;
	    if (ocol > ep->left)
	    {
		++lines_needed;
		ocol = 0;
	    }
	    while (ocol < ep->left)
		++ocol;
	    ocol += crp->printing_width;
	}
	if (!there_was_something_on_this_line)
	    break;
	++lines_needed;
    }
    trace(("lines_needed = %d\n", lines_needed));
    if (lines_needed <= 0)
    {
	trace(("}\n"));
	return;
    }
    if (lines_needed <= 5)
	wide_output_header_need(deeper, lines_needed);

    //
    // Now send the output.
    //
    for (int line = 0; ; ++line)
    {
	//
	// Emit the column contents.
	//
	bool there_was_something_on_this_line = (line < minlines);
	int ocol = 0;
	for (size_t j = 0; j < argc; ++j)
	{
	    emit_ty *ep = &argv[j];
	    if (!ep->content)
		continue;
	    trace(("ep = %08lX;\n", (long)ep));
	    column_row_ty *crp = wide_output_column_get(ep->content, line);
	    if (!crp)
		continue;
	    trace(("crp = %08lX;\n", (long)crp));
	    ++there_was_something_on_this_line;
	    if (!crp->length)
		continue;
	    if (ocol > ep->left)
	    {
		trace(("zzzt, ping!\n"));
		wide_output_putwc(deeper, '\n');
		ocol = 0;
	    }

	    //
	    // We have found something that we are about to
	    // output, emit column headers if necessary.
	    // (Yes, this could happen in the middle of
	    // multi-line output.)
	    //
	    // We don't do this often (hence the (ocol ==
	    // 0) test) because it causes a flush of the
	    // next layer down, and that generally slows
	    // throughput.
	    //
	    if
	    (
		ocol == 0
	    &&
		!this_is_the_header
	    &&
		(
	    	    need_to_emit_headers
		||
		    wide_output_header_is_at_top_of_page(deeper)
		)
	    )
		emit_header();

	    //
	    // Scooch across the to correct column.
	    //
	    while (ocol < ep->left)
	    {
		wide_output_putwc(deeper, ' ');
		++ocol;
	    }

	    //
	    // Write the data, and adjust the column tracking
	    // to match.
	    //
	    wide_output_write(deeper, crp->text, crp->length);
	    ocol += crp->printing_width;
	    trace(("ocol = %d\n", ocol));
	}
	if (!there_was_something_on_this_line)
	    break;
	wide_output_putwc(deeper, '\n');
    }
    trace(("}\n"));
}


void
col_pretty::emit_header()
{
    trace(("col_pretty::emit_header(this = %08lX)\n{\n", (long)this));
    need_to_emit_headers = 0;
    emit_ty *argv = new emit_ty [ncolumns];
    for (size_t j = 0; j < ncolumns; ++j)
    {
	argv[j].content = column[j].header;
	argv[j].left = column[j].left;
    }
    emit(ncolumns, argv, 0, 1);
    delete [] argv;
    need_to_emit_headers = false;
    trace(("}\n"));
}


void
col_pretty::emit_content()
{
    //
    // Flush all of the user input so that it is guaranteed to be
    // in the content buffers.
    //
    trace(("col_pretty::emit_content(this = %08lX)\n{\n", (long)this));
    for (size_t j = 0; j < ncolumns; ++j)
    {
	output_ty *w = column[j].content_filter;
	w->end_of_line();
	w->flush();
    }

    //
    // emit the relevant columns
    //
    emit_ty *argv = new emit_ty [ncolumns];
    for (size_t j = 0; j < ncolumns; ++j)
    {
	argv[j].content = column[j].content;
	argv[j].left = column[j].left;
    }
    emit(ncolumns, argv, 1, 0);
    delete [] argv;

    //
    // reset the content buffers
    //
    for (size_t j = 0; j < ncolumns; ++j)
	wide_output_column_reset(column[j].content);
    trace(("}\n"));
}


static void
delcb_func(output_ty *fp, void *arg)
{
    // called just before a wide output is deleted
    trace(("col_pretty::delcb(fp = %08lX, arg = %08lX)\n{\n",
	(long)fp, (long)arg));
    col_pretty *cp = (col_pretty *)arg;
    cp->delcb(fp);
    trace(("}\n"));
}


void
col_pretty::delcb(output_ty *fp)
{
    for (size_t j = 0; j < ncolumns; ++j)
    {
	column_ty *cp = &column[j];
	if (cp->content_filter == fp)
	{
	    wide_output_delete(cp->header);
	    cp->header = 0;
	    cp->content = 0;
	    cp->content_filter = 0;
	    cp->left = 0;
	    cp->right = 0;
	    break;
	}
    }

    while (ncolumns > 0 && column[ncolumns - 1].content_filter == 0)
	--ncolumns;
    trace(("}\n"));
}


output_ty *
col_pretty::create(int left, int right, const char *ctitle)
{
    //
    // sanity checks on arguments
    //
    trace(("col_pretty::create(this = %08lX, left = %d, right = %d, "
	"ctitle = \"%s\")\n{\n", (long)this, left, right,
	(ctitle ? ctitle : "")));
    if (left < 0)
    {
	if (ncolumns > 0)
    	    left = column[ncolumns - 1].right + 1;
	else
    	    left = 0;
    }
    if (right <= 0)
	right = wide_output_page_width(deeper);
    if (right <= left)
	right = left + 8;

    //
    // make sure we grok enough columns
    //
    if (ncolumns >= ncolumns_max)
    {
	size_t new_max = ncolumns_max * 2 + 4;
	column_ty *new_column = new column_ty [new_max];
	for (size_t j = 0; j < ncolumns; ++j)
	    new_column[j] = column[j];
	delete [] column;
	column = new_column;
	ncolumns_max = new_max;
    }

    //
    // allocate storage for the column content
    //
    column_ty *cp = &column[ncolumns++];
    trace(("left = %d;\n", left));
    trace(("right = %d;\n", right));
    trace(("width = %d;\n", right - left));
    int paglen = wide_output_page_length(deeper);
    cp->content = wide_output_column_open(right - left, paglen);
    cp->left = left;
    cp->right = right;

    //
    // We need to stash the column title specified.
    //
    cp->header = wide_output_column_open(right - left, paglen);
    if (ctitle && *ctitle)
    {
	int nlines = paglen / 2;
	if (nlines < 1)
	    nlines = 1;
	wide_output_ty *fp3 = wide_output_head_open(cp->header, 0, nlines);
	wide_output_ty *fp4 = wide_output_wrap_open(fp3, 1, -1);
	wide_output_ty *fp5 = wide_output_expand_open(fp4, 1);
	wide_output_put_cstr(fp5, ctitle);
	wide_output_end_of_line(fp5);
	wide_output_delete(fp5);

	//
	// A new column with a header implies we need to emit
	// the headers again.
	//
	need_to_emit_headers = 1;
    }

    //
    // What the client of the interface sees is a de-tabbed and wrapped
    // filter into the column content.
    //
    cp->content_filter =
	new output_to_wide_ty
	(
	    wide_output_expand_open
	    (
		wide_output_wrap_open(cp->content, 1, right - left),
		1
	    ),
	    true
	);
    cp->content_filter->delete_callback(delcb_func, this);
    trace(("return %08lX;\n", (long)cp->content_filter));
    trace(("}\n"));
    return cp->content_filter;
}


void
col_pretty::eoln()
{
    trace(("col_pretty::eoln(this = %08lX)\n{\n", (long)this));

    //
    // Headings are emitted at the top of the page, and also when
    // new columns are added if they have headers.
    //
    // But there is a catch 22: the automatic page headers are
    // emitted when the first character of the new page is written,
    // if we were to unconditionally emit them here, they would be
    // emitted twice in some cases.
    //
    if
    (
	need_to_emit_headers
    &&
	!wide_output_header_is_at_top_of_page(deeper)
    &&
	option_page_headers_get()
    )
	emit_header();

    //
    // Now emit the content.
    //
    emit_content();
    trace(("}\n"));
}


void
col_pretty::eject()
{
    trace(("col_pretty::eject(this = %08lX)\n{\n", (long)this));
    wide_output_header_eject(deeper);
    trace(("}\n"));
}


void
col_pretty::need(int n)
{
    trace(("col_pretty::need(this = %08lX, n = %d)\n{\n", (long)this, n));
    wide_output_header_need1(deeper, n);
    trace(("}\n"));
}


void
col_pretty::title(const char *s1, const char *s2)
{
    trace(("col_pretty::title(this = %08lX, s1 = \"%s\", s2 = \"%s\")\n{\n",
	(long)this, (s1 ? s1 : ""), (s2 ? s2 : "")));
    wide_output_header_title(deeper, s1, s2);
    trace(("}\n"));
}


col_pretty::column_ty::~column_ty()
{
}


col_pretty::column_ty::column_ty() :
    header(0),
    content(0),
    content_filter(0),
    left(0),
    right(0)
{
}


col_pretty::column_ty::column_ty(const column_ty &arg) :
    header(arg.header),
    content(arg.content),
    content_filter(arg.content_filter),
    left(arg.left),
    right(arg.right)
{
}


col_pretty::column_ty &
col_pretty::column_ty::operator=(const column_ty &arg)
{
    if (&arg != this)
    {
    	header = arg.header;
    	content = arg.content;
    	content_filter = arg.content_filter;
    	left = arg.left;
    	right = arg.right;
    }
    return *this;
}


void
col_pretty::flush()
{
    wide_output_flush(deeper);
}
