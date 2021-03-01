/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to manipulate unformatteds
 */

#include <col/unformatted.h>
#include <col/private.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <output/to_wide.h>
#include <trace.h>
#include <wide_output/column.h>
#include <wide_output/expand.h>


typedef struct column_ty column_ty;
struct column_ty
{
    wide_output_ty  *content; /* actually wide_output_column_ty */
    output_ty	    *content_filter;
};

typedef struct col_unformatted_ty col_unformatted_ty;
struct col_unformatted_ty
{
    col_ty	    inherited;
    wide_output_ty  *deeper;
    int		    delete_on_close;

    wchar_t	    separator;

    size_t	    ncolumns;
    size_t	    ncolumns_max;
    column_ty	    *column;
};


/*
 * NAME
 *	destructor
 *
 * SYNOPSIS
 *	void destructor(col_ty *);
 *
 * DESCRIPTION
 *	The destructor function is used to release resources held by
 *	this instance.
 */

static void
destructor(col_ty *fp)
{
    col_unformatted_ty *this_thing;
    size_t	    j;

    trace(("col_unformatted::destructor(fp = %08lX)\n{\n", (long)fp));
    this_thing = (col_unformatted_ty *)fp;
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	column_ty	*cp;

	/* The delcb() will do all the work. */
	cp = &this_thing->column[j];
	if (cp->content_filter)
	    output_delete(cp->content_filter);
    }
    if (this_thing->column)
	mem_free(this_thing->column);
    if (this_thing->delete_on_close)
	wide_output_delete(this_thing->deeper);
    trace(("}\n"));
}


/*
 * NAME
 *	delcb
 *
 * SYNOPSIS
 *	void delcb(wide_output_ty *, void *);
 *
 * DESCRIPTION
 *	The delcb function is used to clear out references to deleted
 *	columns.  This function is registered as a callback with
 *	wide_output_callback when the content filter is created.
 */

static void
delcb(output_ty *fp, void *arg)
{
    col_unformatted_ty *this_thing;
    size_t	    j;

    /* called just before a wide output is deleted */
    trace(("col_unformatted::delcb(fp = %08lX, arg = %08lX)\n{\n",
	(long)fp, (long)arg));
    this_thing = (col_unformatted_ty *)arg;
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	column_ty	*cp;

	cp = &this_thing->column[j];
	if (cp->content_filter != fp)
	    continue;
	cp->content = 0;
	cp->content_filter = 0;
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


/*
 * NAME
 *	create
 *
 * SYNOPSIS
 *	wide_output_ty *create(col_ty *colp, int left, int right,
 *		const char *title);
 *
 * DESCRIPTION
 *	The reate function is used to create a new column for this
 *	columnizer.  The column contents will be de-tabbed, but that's
 *	all.  They will not be wrapped or truncated.
 *
 * ARGUMENTS
 *	colp	- the columnizer instance
 *	left	- the left-most column (ignored0
 *	right	- the column past the last column (ignored)
 *	title	- the column title (ignored)
 *
 * RETURNS
 *	output_ty *; a pointer to an output stream.
 *	Use output_delete when you are done with it.
 */

static output_ty *
create(col_ty *fp, int left, int right, const char *title)
{
    col_unformatted_ty *this_thing;
    column_ty	    *cp;
    wide_output_ty  *fp4;

    /*
     * we ignore the left, right and title arguments.
     */
    trace(("col_unformatted::create(fp = %08lX, left = %d, right = %d, "
	"title = %08lX)\n{\n", (long)fp, left, right, (long)title));
    left = 0;
    right = 0;
    title = 0;

    /*
     * make sure we grok enough columns
     */
    this_thing = (col_unformatted_ty *)fp;
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
	    cp->content = 0;
	}
    }

    /*
     * allocate storage for the column content
     */
    trace(("mark\n"));
    cp = &this_thing->column[this_thing->ncolumns++];
    cp->content =
	wide_output_column_open
	(
    	    wide_output_page_width(this_thing->deeper),
    	    wide_output_page_length(this_thing->deeper)
	);

    /*
     * What the client of the interface sees is a de-tabbed
     * filter into the column content.
     * It isn't wrapped, and it isn't truncated.
     */
    trace(("mark\n"));
    fp4 = wide_output_expand_open(cp->content, 1);
    cp->content_filter = output_to_wide_open(fp4, 1);
    output_delete_callback(cp->content_filter, delcb, this_thing);
    trace(("return %08lX;\n", (long)cp->content_filter));
    trace(("}\n"));
    return cp->content_filter;
}


static void
title(col_ty *fp, const char *s1, const char *s2)
{
    trace(("col_unformatted::title(fp = %08lX)\n{\n", (long)fp));
    /* do nothing */
    trace(("}\n"));
}


/*
 * NAME
 *	eoln
 *
 * SYNOPSIS
 *	void eoln(col_ty *);
 *
 * DESCRIPTION
 *	The eoln function is used to print a whole line of columns,
 *	once all of the columns have been set (they need not be set in
 *	any particular order).
 *
 *	In this unformatted style, each non-empty column is emitted,
 *	with a speace between them.
 */

static void
eoln(col_ty *fp)
{
    col_unformatted_ty *this_thing;
    size_t	    j;
    int		    col;

    /*
     * Send the first line of each column, only.
     */
    trace(("col_unformatted::eoln(fp = %08lX)\n{\n", (long)fp));
    this_thing = (col_unformatted_ty *)fp;
    col = 0;
    for (j = 0; j < this_thing->ncolumns; ++j)
    {
	column_ty	*cp;
	column_row_ty	*crp;
	wchar_t		*wp;
	size_t		wp_len;

	/*
	 * flush the output first
	 */
	cp = &this_thing->column[j];
	trace(("cp = %08lX;\n", (long)cp));
	if (!cp->content_filter)
	    continue;
	output_end_of_line(cp->content_filter);
	output_flush(cp->content_filter);

	/*
	 * Find the text for this column.
	 */
	assert(cp->content);
	crp = wide_output_column_get(cp->content, 0);
	trace(("crp = %08lX;\n", (long)crp));
	wp = 0;
	wp_len = 0;
	if (crp)
	{
	    wp = crp->text;
	    wp_len = crp->length;
	}

	/*
	 * Trim off leading and trailing spaces.
	 */
	while (wp_len > 0 && *wp == (wchar_t)' ')
	{
	    ++wp;
	    --wp_len;
	}
	while (wp_len > 0 && wp[wp_len - 1] == (wchar_t)' ')
	    --wp_len;

	/*
	 * If there is anything to print, produce a separator
	 * and the text.  If there is not, but the separator is
	 * not a space (e.g. a comma, for CSV files) emit the
	 * separator anyway.
	 */
	if (wp_len > 0)
	{
	    if (col++)
	       	wide_output_putwc(this_thing->deeper, this_thing->separator);
	    wide_output_write(this_thing->deeper, wp, wp_len);
	}
	else if (this_thing->separator != (wchar_t)' ' && col++)
	    wide_output_putwc(this_thing->deeper, this_thing->separator);

	/*
	 * reset content buffers
	 */
	wide_output_column_reset(cp->content);
    }
    wide_output_putwc(this_thing->deeper, (wchar_t)'\n');
    trace(("}\n"));
}


/*
 * NAME
 *	eject
 *
 * SYNOPSIS
 *	void eject(col_ty *);
 *
 * DESCRIPTION
 *	The eject function is used to move to a new page, if we had
 *	pages, which the unformatted style does not, so it simply throws
 *	a blank line.
 */

static void
eject(col_ty *fp)
{
    col_unformatted_ty *this_thing;

    trace(("col_unformatted::eject(fp = %08lX)\n{\n", (long)fp));
    this_thing = (col_unformatted_ty *)fp;
    wide_output_putwc(this_thing->deeper, '\n');
    trace(("}\n"));
}


/*
 * NAME
 *	need
 *
 * SYNOPSIS
 *	void need(col_ty *colp, int nlines);
 *
 * DESCRIPTION
 *	The need function is used to throw a new page if the requested
 *	number of lines is not available.  The unformated style has no
 *	pages, so this function does nothing.
 */

static void
need(col_ty *fp, int n)
{
    trace(("col_unformatted::need(fp = %08lX, nlines = %d)\n{\n", (long)fp, n));
    /* do nothing, the page is infinitely long */
    trace(("}\n"));
}


static col_vtbl_ty vtbl =
{
    sizeof(col_unformatted_ty),
    destructor,
    create,
    title,
    eoln,
    eject,
    need,
    "unformatted",
};


/*
 * NAME
 *	col_unformatted_open
 *
 * SYNOPSIS
 *	col_ty *col_unformatted_open(wide_output_ty *deeper,
 *		int delete_on_close);
 *
 * DESCRIPTION
 *	The col_unformatted_open function is used to create a new
 *	unformatted columnizer.
 *
 * ARGUMENTS
 *	deeper	- the output stream to write the results onto.
 *	delete_on_close - if true, delete "deeper" when we ourselves
 *		are deleted.
 *
 * RETURNS
 *	col_ty *; a pointer to a columnizer instance is dynamic memory.
 *	Use col_close when you are done with it.
 */

col_ty *
col_unformatted_open(wide_output_ty *deeper, int delete_on_close)
{
    col_ty	    *result;
    col_unformatted_ty *this_thing;

    trace(("col_unformatted::new(deeper = %08lX)\n{\n", (long)deeper));
    result = col_new(&vtbl);
    this_thing = (col_unformatted_ty *)result;
    this_thing->deeper = deeper;
    this_thing->delete_on_close = delete_on_close;
    this_thing->ncolumns = 0;
    this_thing->ncolumns_max = 0;
    this_thing->column = 0;
    this_thing->separator = (wchar_t)' ';
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}
