//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2007 Peter Miller
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
// MANIFEST: functions to manipulate unformatteds
//

#include <common/error.h> // for assert
#include <common/trace.h>
#include <libaegis/col/unformatted.h>
#include <libaegis/output/to_wide.h>
#include <libaegis/wide_output/column.h>
#include <libaegis/wide_output/expand.h>


col_unformatted::~col_unformatted()
{
    trace(("col_unformatted::~col_unformatted(this = %08lX)\n{\n", (long)this));
    for (size_t j = 0; j < ncolumns; ++j)
    {
	// The delcb() will do all the work.
	column_ty *cp = &column[j];
	if (cp->content_filter)
	{
	    delete cp->content_filter;
	    cp->content_filter = 0;
	}
    }
    delete [] column;
    if (delete_on_close)
	wide_output_delete(deeper);
    deeper = 0;
    trace(("}\n"));
}


col_unformatted::col_unformatted(wide_output_ty *arg1, bool arg2) :
    deeper(arg1),
    delete_on_close(arg2),
    separator((wchar_t)' '),
    ncolumns(0),
    ncolumns_max(0),
    column(0)
{
    trace(("col_unformatted::new(deeper = %08lX)\n{\n", (long)deeper));
    trace(("}\n"));
}


/**
  * The delcb_func function is used to clear out references to
  * deleted columns.  This function is registered as a callback with
  * wide_output_callback when the content filter is created.
  */

static void
delcb_func(output_ty *fp, void *arg)
{
    // called just before a wide output is deleted
    trace(("col_unformatted::delcb(fp = %08lX, arg = %08lX)\n{\n",
	(long)fp, (long)arg));
    col_unformatted *cp = (col_unformatted *)arg;
    cp->delcb(fp);
    trace(("}\n"));
}


void
col_unformatted::delcb(output_ty *fp)
{
    for (size_t j = 0; j < ncolumns; ++j)
    {
	column_ty *cp = &column[j];
	if (cp->content_filter == fp)
	{
	    cp->content = 0;
	    cp->content_filter = 0;
	    break;
	}
    }

    while (ncolumns > 0 && column[ncolumns - 1].content_filter == 0)
	--ncolumns;
    trace(("}\n"));
}


output_ty *
col_unformatted::create(int left, int right, const char *ctitle)
{
    //
    // we ignore the left, right and ctitle arguments.
    //
    trace(("col_unformatted::create(this = %08lX, left = %d, right = %d, "
	"ctitle = %08lX)\n{\n", (long)this, left, right, (long)ctitle));
    left = 0;
    right = 0;
    ctitle = 0;

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
	wide_output_column_open
	(
    	    wide_output_page_width(deeper),
    	    wide_output_page_length(deeper)
	);

    //
    // What the client of the interface sees is a de-tabbed
    // filter into the column content.
    // It isn't wrapped, and it isn't truncated.
    //
    trace(("mark\n"));
    wide_output_ty *fp4 = wide_output_expand_open(cp->content, 1);
    cp->content_filter = new output_to_wide_ty(fp4, true);
    cp->content_filter->delete_callback(delcb_func, this);
    trace(("return %08lX;\n", (long)cp->content_filter));
    trace(("}\n"));
    return cp->content_filter;
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
	trace(("cp = %08lX;\n", (long)cp));
	if (!cp->content_filter)
	    continue;
	cp->content_filter->end_of_line();
	cp->content_filter->flush();

	//
	// Find the text for this column.
	//
	assert(cp->content);
	column_row_ty *crp = wide_output_column_get(cp->content, 0);
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
	while (wp_len > 0 && *wp == (wchar_t)' ')
	{
	    ++wp;
	    --wp_len;
	}
	while (wp_len > 0 && wp[wp_len - 1] == (wchar_t)' ')
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
	       	wide_output_putwc(deeper, separator);
	    wide_output_write(deeper, wp, wp_len);
	}
	else if (separator != (wchar_t)' ' && colnum++)
	    wide_output_putwc(deeper, separator);

	//
	// reset content buffers
	//
	wide_output_column_reset(cp->content);
    }
    wide_output_putwc(deeper, (wchar_t)'\n');
    trace(("}\n"));
}


void
col_unformatted::eject()
{
    trace(("col_unformatted::eject(this = %08lX)\n{\n", (long)this));
    wide_output_putwc(deeper, '\n');
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
    content = 0;
    content_filter = 0;
}


col_unformatted::column_ty::column_ty() :
    content(0),
    content_filter(0)
{
}


col_unformatted::column_ty::column_ty(const column_ty &arg) :
    content(arg.content),
    content_filter(arg.content_filter)
{
}


col_unformatted::column_ty &
col_unformatted::column_ty::operator=(const column_ty &arg)
{
    if (&arg != this)
    {
	content = 0;
	content_filter = 0;
	content = arg.content;
	content_filter = arg.content_filter;
    }
    return *this;
}


void
col_unformatted::flush()
{
    wide_output_flush(deeper);
}
