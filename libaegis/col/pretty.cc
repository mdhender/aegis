//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2005 Peter Miller;
//	All rights reserved.
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

#include <libaegis/col/pretty.h>
#include <libaegis/col/private.h>
#include <common/error.h> // for assert
#include <common/mem.h>
#include <libaegis/option.h>
#include <libaegis/output/to_wide.h>
#include <common/trace.h>
#include <libaegis/wide_output/column.h>
#include <libaegis/wide_output/expand.h>
#include <libaegis/wide_output/head.h>
#include <libaegis/wide_output/header.h>
#include <libaegis/wide_output/unexpand.h>
#include <libaegis/wide_output/wrap.h>


struct column_ty
{
    wide_output_ty  *header; // actually wide_output_column_ty
    wide_output_ty  *content; // actually wide_output_column_ty
    output_ty	    *content_filter;
    int		    left;
    int		    right;
};

struct col_pretty_ty
{
    col_ty	    inherited;
    wide_output_ty  *deeper;
    int		    need_to_emit_headers;

    size_t	    ncolumns;
    size_t	    ncolumns_max;
    column_ty	    *column;
};


//
// NAME
//	destructor
//
// SYNOPSIS
//	void destructor(col_ty *);
//
// DESCRIPTION
//	The destructor function is used to release resources held by
//	this columnizing instance.
//

static void
destructor(col_ty *fp)
{
    col_pretty_ty   *this_thing;
    size_t	    j;

    trace(("col_pretty::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (col_pretty_ty *)fp;
    trace(("ncolumns = %d\n", (int)this_thing->ncolumns));
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	column_ty	*cp;

	// The delcb() will do all the work.
	cp = &this_thing->column[j];
	trace(("cp = %08lX\n", (long)cp));
	if (cp->content_filter)
	{
	    delete cp->content_filter;
	    cp->content_filter = 0;
	}
    }
    trace(("column = %08lX\n", (long)this_thing->column));
    if (this_thing->column)
	mem_free(this_thing->column);
    trace(("deeper = %08lX\n", (long)this_thing->deeper));
    wide_output_delete(this_thing->deeper);
    trace(("}\n"));
}


struct emit_ty
{
    wide_output_ty  *content;
    int		    left;
};


static void emit_header(col_pretty_ty *); // forward


//
// NAME
//	emit
//
// SYNOPSIS
//	void emit(col_pretty_ty *this_thing, size_t argc, emit_ty *argv));
//
// DESCRIPTION
//	The emit function is used to emit the given values of text out
//	of the deeper output stream.  The emitting is factored into this
//	common function, rather than reproduce the code twice, once
//	for the headers and again for the content.
//

static void
emit(col_pretty_ty *this_thing, size_t argc, emit_ty *argv, int minlines,
    int this_is_the_header)
{
    int		    line;
    int		    need;

    //
    // Figure out how many lines of output we will produce, and ask
    // for a page eject if that many lines aren't left on the page.
    //
    trace(("col_pretty::emit(this_thing = %08lX)\n{\n", (long)this_thing));
    need = 0;
    for (line = 0; ; ++line)
    {
	int		there_was_something_on_this_thing_line;
	int		ocol;
	int		j;

	there_was_something_on_this_thing_line = (line < minlines);
	ocol = 0;
	for (j = 0; j < (int)argc; ++j)
	{
	    emit_ty         *ep;
	    column_row_ty   *crp;

	    ep = &argv[j];
	    if (!ep->content)
		continue;
	    trace(("ep = %08lX;\n", (long)ep));
	    crp = wide_output_column_get(ep->content, line);
	    if (!crp)
		continue;
	    trace(("crp = %08lX;\n", (long)crp));
	    ++there_was_something_on_this_thing_line;
	    if (!crp->length)
		continue;
	    if (ocol > ep->left)
	    {
		++need;
		ocol = 0;
	    }
	    while (ocol < ep->left)
		++ocol;
	    ocol += crp->printing_width;
	}
	if (!there_was_something_on_this_thing_line)
	    break;
	++need;
    }
    trace(("need = %d\n", need));
    if (need <= 0)
    {
	trace(("}\n"));
	return;
    }
    if (need <= 5)
	wide_output_header_need(this_thing->deeper, need);

    //
    // Now send the output.
    //
    for (line = 0; ; ++line)
    {
	int             there_was_something_on_this_thing_line;
	int		ocol;
	int		j;

	//
	// Emit the column contents.
	//
	there_was_something_on_this_thing_line = (line < minlines);
	ocol = 0;
	for (j = 0; j < (int)argc; ++j)
	{
	    emit_ty         *ep;
	    column_row_ty   *crp;

	    ep = &argv[j];
	    if (!ep->content)
		continue;
	    trace(("ep = %08lX;\n", (long)ep));
	    crp = wide_output_column_get(ep->content, line);
	    if (!crp)
		continue;
	    trace(("crp = %08lX;\n", (long)crp));
	    ++there_was_something_on_this_thing_line;
	    if (!crp->length)
		continue;
	    if (ocol > ep->left)
	    {
		trace(("zzzt, ping!\n"));
		wide_output_putwc(this_thing->deeper, '\n');
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
	    	    this_thing->need_to_emit_headers
		||
		    wide_output_header_is_at_top_of_page(this_thing->deeper)
		)
	    )
		emit_header(this_thing);

	    //
	    // Scooch across the to correct column.
	    //
	    while (ocol < ep->left)
	    {
		wide_output_putwc(this_thing->deeper, ' ');
		++ocol;
	    }

	    //
	    // Write the data, and adjust the column tracking
	    // to match.
	    //
	    wide_output_write(this_thing->deeper, crp->text, crp->length);
	    ocol += crp->printing_width;
	    trace(("ocol = %d\n", ocol));
	}
	if (!there_was_something_on_this_thing_line)
	    break;
	wide_output_putwc(this_thing->deeper, '\n');
    }
    trace(("}\n"));
}


//
// NAME
//	emit_header
//
// SYNOPSIS
//	void emit_header(col_pretty_ty *this_thing);
//
// DESCRIPTION
//	The emit_header function is used to emit column headers when
//	necessary.  It clears the flag which controls emitting the
//	headers, too.
//

static void
emit_header(col_pretty_ty *this_thing)
{
    emit_ty	    *argv;
    size_t	    j;

    trace(("col_pretty::emit_header(this = %08lX)\n{\n", (long)this_thing));
    this_thing->need_to_emit_headers = 0;
    argv = (emit_ty *)mem_alloc(this_thing->ncolumns * sizeof(emit_ty));
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	argv[j].content = this_thing->column[j].header;
	argv[j].left = this_thing->column[j].left;
    }
    emit(this_thing, this_thing->ncolumns, argv, 0, 1);
    mem_free(argv);
    this_thing->need_to_emit_headers = 0;
    trace(("}\n"));
}


//
// NAME
//	emit_content
//
// SYNOPSIS
//	void emit_content(col_pretty_ty *this_thing);
//
// DESCRIPTION
//	The emit_content function is used to generate the output, given
//	the content written inforctly via the wide output streams privide
//	by create (below).
//
//	Usually, the column headers are handled by the top-of-page
//	callback, however if new columns with headers are created in the
//	middile of the page, this function also causes the new column
//	headings to be emitted, before the content.
//

static void
emit_content(col_pretty_ty *this_thing)
{
    emit_ty	    *argv;
    size_t	    j;

    //
    // Flush all of the user input so that it is guaranteed to be
    // in the content buffers.
    //
    trace(("col_pretty::emit_content(this = %08lX)\n{\n", (long)this_thing));
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	output_ty	*w;

	w = this_thing->column[j].content_filter;
	w->end_of_line();
	w->flush();
    }

    //
    // emit the relevant columns
    //
    argv = (emit_ty *)mem_alloc(this_thing->ncolumns * sizeof(emit_ty));
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	argv[j].content = this_thing->column[j].content;
	argv[j].left = this_thing->column[j].left;
    }
    emit(this_thing, this_thing->ncolumns, argv, 1, 0);
    mem_free(argv);

    //
    // reset the content buffers
    //
    for (j = 0; j < this_thing->ncolumns; ++j)
	wide_output_column_reset(this_thing->column[j].content);
    trace(("}\n"));
}


//
// NAME
//	delcb
//
// SYNOPSIS
//	void delcb(output_ty *fp, void *aux));
//
// DESCRIPTION
//	The delcb function is used to notify us that one of the output
//	streams (which indirectly fills out column contents) has been
//	deleted.  This allows us to stop remembering it, and thus not
//	de-reference dangling pointers.
//

static void
delcb(output_ty *fp, void *arg)
{
    col_pretty_ty   *this_thing;
    size_t	    j;

    // called just before a wide output is deleted
    trace(("col_pretty::delcb(fp = %08lX, arg = %08lX)\n{\n",
	(long)fp, (long)arg));
    this_thing = (col_pretty_ty *)arg;
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	column_ty       *cp;

	cp = &this_thing->column[j];
	if (cp->content_filter != fp)
	    continue;
	wide_output_delete(cp->header);
	cp->header = 0;
	cp->content = 0;
	cp->content_filter = 0;
	cp->left = 0;
	cp->right = 0;
	break;
    }

    while
    (
	this_thing->ncolumns > 0
    &&
	this_thing->column[this_thing->ncolumns - 1].content_filter == 0
    )
	this_thing->ncolumns--;
    trace(("}\n"));
}


//
// NAME
//	create
//
// SYNOPSIS
//	output_ty *create(col_ty *colp, int left, int right,
//		const char *title));
//
// DESCRIPTION
//	The create function is used to create a new column instance.
//	It will be de-tabbed and wrapped into the column papameters given.
//	When output, it will be positioned as given.
//
// ARGUMENTS
//	colp	- the columnizing instance
//	left	- the left-most column
//	right	- the rightmost column, plus one (i.e. right-left=width)
//	title	- thew column title to use (NULL means none)
//
// RETURNS
//	output_ty *; a pointer to an output stream.
//	use output_delete when you are done with it.
//

static output_ty *
create(col_ty *fp, int left, int right, const char *title)
{
    col_pretty_ty   *this_thing;
    wide_output_ty  *fp3;
    wide_output_ty  *fp4;
    column_ty	    *cp;
    int		    paglen;

    //
    // sanity checks on arguments
    //
    trace(("col_pretty::create(fp = %08lX, left = %d, right = %d, "
	"title = \"%s\")\n{\n", (long)fp, left, right, (title ? title : "")));
    this_thing = (col_pretty_ty *)fp;
    if (left < 0)
    {
	if (this_thing->ncolumns > 0)
    	    left = this_thing->column[this_thing->ncolumns - 1].right + 1;
	else
    	    left = 0;
    }
    if (right <= 0)
	right = wide_output_page_width(this_thing->deeper);
    if (right <= left)
	right = left + 8;

    //
    // make sure we grok enough columns
    //
    if (this_thing->ncolumns >= this_thing->ncolumns_max)
    {
	size_t		nbytes;
	int		old;

	old = this_thing->ncolumns_max;
	this_thing->ncolumns_max = this_thing->ncolumns_max * 2 + 4;
	nbytes = this_thing->ncolumns_max * sizeof(this_thing->column[0]);
	this_thing->column =
            (column_ty *)mem_change_size(this_thing->column, nbytes);
	while (old < (int)this_thing->ncolumns_max)
	{
	    cp = &this_thing->column[old++];
	    cp->header = 0;
	    cp->content = 0;
	    cp->content_filter = 0;
	    cp->left = 0;
	    cp->right = 0;
	}
    }

    //
    // allocate storage for the column content
    //
    cp = &this_thing->column[this_thing->ncolumns++];
    trace(("left = %d;\n", left));
    trace(("right = %d;\n", right));
    trace(("width = %d;\n", right - left));
    paglen = wide_output_page_length(this_thing->deeper);
    cp->content = wide_output_column_open(right - left, paglen);
    cp->left = left;
    cp->right = right;

    //
    // We need to stash the column title specified.
    //
    cp->header = wide_output_column_open(right - left, paglen);
    if (title && *title)
    {
	wide_output_ty	*fp5;
	int		nlines;

	nlines = paglen / 2;
	if (nlines < 1)
	    nlines = 1;
	fp3 = wide_output_head_open(cp->header, 0, nlines);
	fp4 = wide_output_wrap_open(fp3, 1, -1);
	fp5 = wide_output_expand_open(fp4, 1);
	wide_output_put_cstr(fp5, title);
	wide_output_end_of_line(fp5);
	wide_output_delete(fp5);

	//
	// A new column with a header implies we need to emit
	// the headers again.
	//
	this_thing->need_to_emit_headers = 1;
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
    cp->content_filter->delete_callback(delcb, this_thing);
    trace(("return %08lX;\n", (long)cp->content_filter));
    trace(("}\n"));
    return cp->content_filter;
}


//
// NAME
//	eoln
//
// SYNOPSIS
//	void eoln(col_ty *colp);
//
// DESCRIPTION
//	The eoln function is used to print a whole line of columns,
//	once all of the columns have been set (they need not be set in
//	any particular order).
//
//	In this pretty style, each column is printed with the given
//	left margin, by inserting the necessary number of spaces..
//	(The columns have already been wrapped by the time we arive here.)
//	If the current output column exceeds the desired column, a newline
//	is whrown and then spaces emitted to move to the desired column.
//	Blank columns are ignored.
//
//	Runs of spaces are converted to tabs "down stream" in the deeper
//	output stream, and need not (must not) be inserted here.
//

static void
eoln(col_ty *fp)
{
    col_pretty_ty   *this_thing;

    trace(("col_pretty::eoln(fp = %08lX)\n{\n", (long)fp));
    this_thing = (col_pretty_ty *)fp;

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
	this_thing->need_to_emit_headers
    &&
	!wide_output_header_is_at_top_of_page(this_thing->deeper)
    &&
	option_page_headers_get()
    )
	emit_header(this_thing);

    //
    // Now emit the content.
    //
    emit_content(this_thing);
    trace(("}\n"));
}


//
// NAME
//	eject
//
// SYNOPSIS
//	void eject(col_ty *colp);
//
// DESCRIPTION
//	The eject function is used to move to the top of the next page.
//	We don't actually do it, we let the "down stream" header filter
//	do it.
//

static void
eject(col_ty *fp)
{
    col_pretty_ty   *this_thing;

    trace(("col_pretty::eject(fp = %08lX)\n{\n", (long)fp));
    this_thing = (col_pretty_ty *)fp;
    wide_output_header_eject(this_thing->deeper);
    trace(("}\n"));
}


//
// NAME
//	need
//
// SYNOPSIS
//	void need(col_ty *colp, in nlines);
//
// DESCRIPTION
//	The need function is used to move to the top of the next page if
//	there is less than "nlines" left on the current page.  We don't
//	actually do it, we let the "down stream" header filter do it.
//

static void
need(col_ty *fp, int n)
{
    col_pretty_ty   *this_thing;

    trace(("col_pretty::need(fp = %08lX, n = %d)\n{\n", (long)fp, n));
    this_thing = (col_pretty_ty *)fp;
    wide_output_header_need1(this_thing->deeper, n);
    trace(("}\n"));
}


//
// NAME
//	title
//
// SYNOPSIS
//	void title(col_ty *colp, const char *line1, const char *line2);
//
// DESCRIPTION
//	The title function is used to set the page titles for the next
//	page printed.  We don't actually do it, we let the "down stream"
//	header filter do it.
//
// ARGUMENTS
//
// RETURNS
//

static void
title(col_ty *fp, const char *s1, const char *s2)
{
    col_pretty_ty   *this_thing;

    trace(("col_pretty::title(fp = %08lX, s1 = \"%s\", s2 = \"%s\")\n{\n",
	(long)fp, (s1 ? s1 : ""), (s2 ? s2 : "")));
    this_thing = (col_pretty_ty *)fp;
    wide_output_header_title(this_thing->deeper, s1, s2);
    trace(("}\n"));
}


static col_vtbl_ty vtbl =
{
    sizeof(col_pretty_ty),
    destructor,
    create,
    title,
    eoln,
    eject,
    need,
    "pretty",
};


//
// NAME
//	col_pretty_open
//
// SYNOPSIS
//	void col_pretty_open(void);
//
// DESCRIPTION
//	The col_pretty_open function is used to create a new columnized
//	output.  You may open sub-outputs, which will be printed in
//	columns across this output.
//
// ARGUMENTS
//	deeper -
//		where to write out iutput
//	delete_on_close -
//		whether to delete `deeper' when this output itself is deleted.
//
// RETURNS
//	col_ty *; a pointer to a valid col_ty structure.
//

col_ty *
col_pretty_open(wide_output_ty *deeper, int delete_on_close)
{
    col_ty	    *result;
    col_pretty_ty   *this_thing;

    trace(("col_pretty::new(deeper = %08lX)\n{\n", (long)deeper));
    result = col_new(&vtbl);
    this_thing = (col_pretty_ty *)result;
    this_thing->deeper = wide_output_unexpand_open(deeper, delete_on_close, -1);
    if (option_page_headers_get())
	this_thing->deeper = wide_output_header_open(this_thing->deeper, 1);
    this_thing->need_to_emit_headers = 0; // not until one gets create()ed
    this_thing->ncolumns = 0;
    this_thing->ncolumns_max = 0;
    this_thing->column = 0;
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
