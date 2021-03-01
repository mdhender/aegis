//
// aegis - project change supervisor
// Copyright (C) 1999, 2001-2006, 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/mem.h>
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
    trace(("col_pretty::destructor(this = %p)\n{\n", this));
    trace(("ncolumns = %d\n", (int)ncolumns));
    for (size_t j = 0; j < ncolumns; ++j)
    {
        column[j].clear();
    }
    trace(("column = %p\n", column));
    delete [] column;
    trace(("}\n"));
}


static wide_output::pointer
mangle(wide_output::pointer arg)
{
    wide_output::pointer arg2 = wide_output_unexpand::open(arg, -1);
    if (option_page_headers_get())
        arg2 = wide_output_header::open(arg2);
    return arg2;
}


col_pretty::col_pretty(const wide_output::pointer &a_deeper) :
    deeper(mangle(a_deeper)),
    need_to_emit_headers(false),
    ncolumns(0),
    ncolumns_max(0),
    column(0)
{
    trace(("col_pretty::col_pretty(this = %p, deeper = %p)\n{\n",
        this, deeper.get()));
    trace(("}\n"));
}


col::pointer
col_pretty::create(const wide_output::pointer &a_deeper)
{
    return pointer(new col_pretty(a_deeper));
}


void
col_pretty::emit(size_t argc, emit_ty *argv, int minlines,
    bool this_is_the_header)
{
    //
    // Figure out how many lines of output we will produce, and ask
    // for a page eject if that many lines aren't left on the page.
    //
    trace(("col_pretty::emit(this = %p)\n{\n", this));
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
            trace(("ep = %p;\n", ep));
            column_row_ty *crp = ep->content->get(line);
            if (!crp)
                continue;
            trace(("crp = %p;\n", crp));
            there_was_something_on_this_line = true;
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
    {
        wide_output_header *hp =
            dynamic_cast<wide_output_header *>(deeper.get());
        if (hp)
            hp->need(lines_needed);
    }

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
            trace(("ep = %p;\n", ep));
            column_row_ty *crp = ep->content->get(line);
            if (!crp)
                continue;
            trace(("crp = %p;\n", crp));
            there_was_something_on_this_line = true;
            if (!crp->length)
                continue;
            if (ocol > ep->left)
            {
                trace(("zzzt, ping!\n"));
                deeper->put_wc(L'\n');
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
                    wide_output_header::is_at_top_of_page(deeper)
                )
            )
                emit_header();

            //
            // Scooch across the to correct column.
            //
            while (ocol < ep->left)
            {
                deeper->put_wc(L' ');
                ++ocol;
            }

            //
            // Write the data, and adjust the column tracking
            // to match.
            //
            deeper->write(crp->text, crp->length);
            ocol += crp->printing_width;
            trace(("ocol = %d\n", ocol));
        }
        if (!there_was_something_on_this_line)
            break;
        deeper->put_wc(L'\n');
    }
    trace(("}\n"));
}


void
col_pretty::emit_header()
{
    trace(("col_pretty::emit_header(this = %p)\n{\n", this));
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
    trace(("col_pretty::emit_content(this = %p)\n{\n", this));
    for (size_t j = 0; j < ncolumns; ++j)
    {
        output::pointer w = column[j].content_filter;
        w->end_of_line();
        w->flush();
    }

    //
    // emit the relevant columns
    //
    int argc = 0;
    emit_ty *argv = new emit_ty [ncolumns];
    for (size_t j = 0; j < ncolumns; ++j)
    {
        if (column[j].content_filter)
        {
            argv[argc].content = column[j].content;
            argv[argc].left = column[j].left;
            ++argc;
        }
    }
    emit(argc, argv, 1, 0);
    delete [] argv;

    //
    // clear the content buffers
    //
    for (size_t j = 0; j < ncolumns; ++j)
        column[j].content->clear_buffers();
    trace(("}\n"));
}


output::pointer
col_pretty::create(int left, int right, const char *ctitle)
{
    //
    // sanity checks on arguments
    //
    trace(("col_pretty::create(this = %p, left = %d, right = %d, "
        "ctitle = \"%s\")\n{\n", this, left, right,
        (ctitle ? ctitle : "")));
    if (left < 0)
    {
        if (ncolumns > 0)
            left = column[ncolumns - 1].right + 1;
        else
            left = 0;
    }
    if (right <= 0)
        right = deeper->page_width();
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
    int paglen = deeper->page_length();
    cp->content = wide_output_column::open(right - left, paglen);
    cp->left = left;
    cp->right = right;

    //
    // We need to stash the column title specified.
    //
    cp->header = wide_output_column::open(right - left, paglen);
    if (ctitle && *ctitle)
    {
        int nlines = paglen / 2;
        if (nlines < 1)
            nlines = 1;
        wide_output::pointer fp3 =
            wide_output_head::open(cp->header, nlines);
        wide_output::pointer fp4 = wide_output_wrap::open(fp3, -1);
        wide_output::pointer fp5 = wide_output_expand::open(fp4);
        fp5->put_cstr(ctitle);
        fp5->end_of_line();

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
        output_to_wide::open
        (
            wide_output_expand::open
            (
                wide_output_wrap::open(cp->content, right - left)
            )
        );
    trace(("return %p;\n", cp->content_filter.get()));
    trace(("}\n"));
    return cp->content_filter;
}


void
col_pretty::forget(const output::pointer &op)
{
    for (size_t j = 0; j < ncolumns; ++j)
    {
        column_ty *cp = &column[j];
        if (cp->content_filter == op)
            cp->clear();
    }
    while (ncolumns > 0 && !column[ncolumns - 1].content_filter)
        --ncolumns;
}


void
col_pretty::eoln()
{
    trace(("col_pretty::eoln(this = %p)\n{\n", this));

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
        !wide_output_header::is_at_top_of_page(deeper)
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
    trace(("col_pretty::eject(this = %p)\n{\n", this));
    wide_output_header::eject(deeper);
    trace(("}\n"));
}


void
col_pretty::need(int n)
{
    trace(("col_pretty::need(this = %p, n = %d)\n{\n", this, n));
    wide_output_header::need1(deeper, n);
    trace(("}\n"));
}


void
col_pretty::title(const nstring &s1, const nstring &s2)
{
    trace(("col_pretty::title(this = %p, s1 = %s, s2 = %s)\n{\n",
        this, s1.quote_c().c_str(), s2.quote_c().c_str()));
    wide_output_header::title(deeper, s1, s2);
    trace(("}\n"));
}


col_pretty::column_ty::~column_ty()
{
    clear();
}


col_pretty::column_ty::column_ty() :
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
col_pretty::column_ty::clear()
{
    header.reset();
    content.reset();
    content_filter.reset();
    left = 0;
    right = 0;
}


void
col_pretty::flush()
{
    deeper->flush();
}


// vim: set ts=8 sw=4 et :
