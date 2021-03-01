//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/col/unformatted.h>
#include <libaegis/output/to_wide.h>
#include <libaegis/wide_output/column.h>
#include <libaegis/wide_output/expand.h>


col_unformatted::~col_unformatted()
{
    trace(("col_unformatted::~col_unformatted(this = %08lX)\n{\n", (long)this));
    delete [] column;
    trace(("}\n"));
}


col_unformatted::col_unformatted(const wide_output::pointer &a_deeper) :
    deeper(a_deeper),
    separator(L' '),
    ncolumns(0),
    ncolumns_max(0),
    column(0)
{
    trace(("col_unformatted::col_unformatted(this = %08lX, "
        "deeper = %08lX)\n{\n", (long)this, (long)a_deeper.get()));
    trace(("}\n"));
}


col::pointer
col_unformatted::create(const wide_output::pointer &a_deeper)
{
    return pointer(new col_unformatted(a_deeper));
}


output::pointer
col_unformatted::create(int, int, const char *)
{
    //
    // we ignore the left, right and ctitle arguments.
    //
    trace(("col_unformatted::create(this = %08lX)\n{\n", (long)this));

    //
    // make sure we grok enough columns
    //
    if (ncolumns >= ncolumns_max)
    {
	size_t new_max = ncolumns_max * 2 + 16;
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
    trace(("mark\n"));
    column_ty *cp = &column[ncolumns++];
    cp->content =
	wide_output_column::open(deeper->page_width(), deeper->page_length());

    //
    // What the client of the interface sees is a de-tabbed
    // filter into the column content.
    // It isn't wrapped, and it isn't truncated.
    //
    trace(("mark\n"));
    wide_output::pointer fp4 = wide_output_expand::open(cp->content);
    cp->content_filter = output_to_wide::open(fp4);
    trace(("return %08lX;\n", (long)cp->content_filter.get()));
    trace(("}\n"));
    return cp->content_filter;
}


void
col_unformatted::forget(const output::pointer &op)
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
col_unformatted::title(const char *, const char *)
{
    trace(("col_unformatted::title(this = %08lX)\n{\n", (long)this));
    // do nothing
    trace(("}\n"));
}


void
col_unformatted::eoln()
{
    //
    // Send the first line of each column, only.
    //
    trace(("col_unformatted::eoln(this = %08lX)\n{\n", (long)this));
    int colnum = 0;
    for (size_t j = 0; j < ncolumns; ++j)
    {
	//
	// flush the output first
	//
	column_ty *cp = &column[j];
        assert(!cp->content == !cp->content_filter);
	trace(("cp = %08lX;\n", (long)cp));
	if (!cp->content_filter)
	    continue;
	cp->content_filter->end_of_line();
	cp->content_filter->flush();

	//
	// Find the text for this column.
	//
	assert(!!cp->content);
	column_row_ty *crp = cp->content->get(0);
	trace(("crp = %08lX;\n", (long)crp));
	wchar_t *wp = 0;
	size_t wp_len = 0;
	if (crp)
	{
	    wp = crp->text;
	    wp_len = crp->length;
	}

	//
	// Trim off leading and trailing spaces.
	//
	while (wp_len > 0 && *wp == L' ')
	{
	    ++wp;
	    --wp_len;
	}
	while (wp_len > 0 && wp[wp_len - 1] == L' ')
	    --wp_len;

	//
	// If there is anything to print, produce a separator
	// and the text.  If there is not, but the separator is
	// not a space (e.g. a comma, for CSV files) emit the
	// separator anyway.
	//
	if (wp_len > 0)
	{
	    if (colnum++)
	       	deeper->put_wc(separator);
	    deeper->write(wp, wp_len);
	}
	else if (separator != L' ' && colnum++)
	    deeper->put_wc(separator);

	//
	// clear the content buffers
	//
	cp->content->clear_buffers();
    }
    deeper->put_wc(L'\n');
    trace(("}\n"));
}


void
col_unformatted::eject()
{
    trace(("col_unformatted::eject(this = %08lX)\n{\n", (long)this));
    deeper->put_wc(L'\n');
    trace(("}\n"));
}


void
col_unformatted::need(int)
{
    trace(("col_unformatted::need(this = %08lX)\n{\n", (long)this));
    // do nothing, the page is infinitely long
    trace(("}\n"));
}


col_unformatted::column_ty::~column_ty()
{
    clear();
}


col_unformatted::column_ty::column_ty()
{
}


void
col_unformatted::column_ty::clear()
{
    content.reset();
    content_filter.reset();
}


void
col_unformatted::flush()
{
    deeper->flush();
}
