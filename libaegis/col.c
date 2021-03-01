/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1995, 1997-1999, 2001, 2002 Peter Miller;
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
 * MANIFEST: functions for columnized output
 */

#include <col.h>
#include <col/pretty.h>
#include <col/private.h>
#include <col/unformatted.h>
#include <error.h> /* for assert */
#include <mem.h>
#include <option.h>
#include <os.h>
#include <output.h>
#include <output/file.h>
#include <output/pager.h>
#include <output/to_wide.h>
#include <trace.h>
#include <wide_output.h>
#include <wide_output/to_narrow.h>


/*
 * NAME
 *	col_open
 *
 * SYNOPSIS
 *	col_ty *col_open(const char *pathname);
 *
 * DESCRIPTION
 *	The col_open function is used to
 *	open a file for outputting columnar data.
 *
 *	The style of formatting is controlled by the -UNFormatted option.
 *
 * ARGUMENTS
 *	pathname	- name of file to write,
 *			pager is used if NULL pointer given.
 *
 * CAVEAT
 *	All other calls to col_ functions must be bracketed
 *	by col_open and col_close calls.
 */

col_ty *
col_open(filename)
    string_ty	    *filename;
{
    col_ty	    *result;
    output_ty	    *narrow_fp;
    wide_output_ty  *wide_fp;

    /*
     * open a suitable output
     */
    trace(("col_open(filename = %08lX)\n{\n", (long)filename));
    os_become_must_not_be_active();
    if (filename && filename->str_length)
    {
	trace_string(filename->str_text);
	os_become_orig();
	narrow_fp = output_file_text_open(filename);
	os_become_undo();
    }
    else
	narrow_fp = output_pager_open();
    wide_fp = wide_output_to_narrow_open(narrow_fp, 1);

    /*
     * pick a formatting option
     */
    if (option_unformatted_get())
	result = col_unformatted_open(wide_fp, 1);
    else
	result = col_pretty_open(wide_fp, 1);

    /*
     * all done
     */
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *	col_close
 *
 * SYNOPSIS
 *	void col_close(col_ty *);
 *
 * DESCRIPTION
 *	The col_close function is used to
 *	terminate columnar output.
 *
 *	All dynamic memory consumed will be released.
 *
 * CAVEAT
 *	All other calls to col_ functions must be bracketed
 *	by col_open and col_close calls.
 *
 *	Output streams returned by col_create (see below) are no longer
 *	valid after this is called (for this specific col_ty instance,
 *	that is).
 */

void
col_close(fp)
    col_ty	    *fp;
{
    trace(("col_close(fp = %08lX)\n{\n", (long)fp));
    if (fp->vptr->destructor)
    {
	fp->vptr->destructor(fp);
    }
    fp->vptr = 0;
    mem_free(fp);
    trace(("}\n"));
}


/*
 * NAME
 *	col_create
 *
 * SYNOPSIS
 *	output_ty *col_create(col_ty *colp, int left, int right,
 *		const char *heading);
 *
 * DESCRIPTION
 *	The col_create function is used to
 *	specify a range of locations for an output column.
 *
 *	The contents will be de-tabbed and wrapped before output.
 *
 * ARGUMENTS
 *	colp	- the output output to be written to.
 *	left	- the left-hand edge of the column
 *		  If you say -1, it will use the righ-hand side of the
 *		  previous column, plus one.  (Note: NOT the right-most
 *		  column.)
 *	right	- the right-hand edge of the column, plus one
 *		  zero means the rest of the line
 *	title	- the column title to use.  Give NULL pointer for none.
 *
 * RETURNS
 *	output_ty *; a pointer to an output stream to be used to write
 *	into this column.  Use output_delete when you are done with the column.
 */

output_ty *
col_create(fp, min, max, title)
    col_ty	    *fp;
    int		    min;
    int		    max;
    const char	    *title;
{
    output_ty	    *result;

    trace(("col_create(fp = %08lX, left = %d, right = %d, title = %08lX)\n{\n",
	(long)fp, min, max, (long)title));
    assert(fp->vptr->create);
    result = fp->vptr->create(fp, min, max, title);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


/*
 * NAME
 *	col_eoln
 *
 * SYNOPSIS
 *	void col_eoln(col_ty *);
 *
 * DESCRIPTION
 *	The col_eoln function is used to signal that an entire line of
 *	columns has been assembled and that it should be emitted to the
 *	output file or pager.
 *
 *	Columns all receive an implicit newline, and are flushed before
 *	processing.  All of the columns are thus aligned into rows,
 *	even when (due to newlines or wrapping) they would otherwise be
 *	different lengths.
 *
 *	This is the interface used by clients of this subsystem.
 */

void
col_eoln(fp)
    col_ty	    *fp;
{
    trace(("col_eoln(fp = %08lX)\n{\n", (long)fp));
    assert(fp->vptr->eoln);
    fp->vptr->eoln(fp);
    trace(("}\n"));
}


/*
 * NAME
 *	col_title
 *
 * SYNOPSIS
 *	void col_title(col_ty *, const char *s1, const char *s2);
 *
 * DESCRIPTION
 *	The col_title function is used to set the two lines of page
 *	headings.  These are ignored for unformatted output.
 *
 * ARGUMENTS
 *	s1	- first line of heading, NULL means blank
 *	s2	- second line of heading, NULL means blank
 */

void
col_title(fp, s1, s2)
    col_ty	    *fp;
    const char	    *s1;
    const char	    *s2;
{
    trace(("col_title(fp = %08lX, s1 = %08lX, s2 = %08lX)\n{\n",
	(long)fp, (long)s1, (long)s2));
    assert(fp->vptr->title);
    fp->vptr->title(fp, s1, s2);
    trace(("}\n"));
}


/*
 * NAME
 *	col_eject
 *
 * SYNOPSIS
 *	void col_eject(col_ty *);
 *
 * DESCRIPTION
 *	The col_eject function is used to move to the top of the next page,
 *	if we aren't there already.
 */

void
col_eject(fp)
    col_ty	    *fp;
{
    trace(("col_eject(fp = %08lX)\n{\n", (long)fp));
    assert(fp->vptr->eject);
    fp->vptr->eject(fp);
    trace(("}\n"));
}


/*
 * NAME
 *	col_need
 *
 * SYNOPSIS
 *	void col_need(col_ty *, int n);
 *
 * DESCRIPTION
 *	The col_need function is used to numinate that
 *	a number of lines is needed before the end of the page.
 *	If the lines are available, a blank line is emitted,
 *	otherwise a new page is thrown.
 *
 * ARGUMENTS
 *	n	- the number of lines,
 *		  not counting the blank line.
 */

void
col_need(fp, n)
    col_ty	    *fp;
    int		    n;
{
    trace(("col_need(fp = %08lX, n = %d)\n{\n", (long)fp, n));
    assert(fp->vptr->need);
    fp->vptr->need(fp, n);
    trace(("}\n"));
}
