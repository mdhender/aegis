//
//	aegis - project change supervisor
//	Copyright (C) 1991-1993, 2001, 2002, 2004-2006, 2008 Peter Miller.
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

#ifndef COL_H
#define COL_H

#include <common/main.h>
#include <libaegis/output.h>

struct string_ty; // forward

/**
  * The col class is used to represent an output capable of hosting
  * multi-column output.
  */
class col
{
public:
    typedef aegis_shared_ptr<col> pointer;

    /**
      * The destructor.
      */
    virtual ~col();

protected:
    /**
      * The default constructor.
      */
    col();

public:
    /**
      * The open class method is a factory used to open a file for
      * outputting columnar data.
      *
      * The style of formatting is controlled by the -UNFormatted option.
      *
      * Use the delete operator when you are done with it.
      *
      * @param filename
      *     name of file to write,
      *     pager is used if NULL pointer given.
      */
    static col::pointer open(struct string_ty *filename);

    /**
      * The create method is used to specify a range of locations for an
      * output column.
      *
      * The contents will be de-tabbed and wrapped before output.
      *
      * @param left
      *     the left-hand edge of the column.
      *     If you say -1, it will use the righ-hand side of the
      *     previous column, plus one.  (Note: NOT the right-most
      *     column.)
      * @param right
      *     the right-hand edge of the column, plus one.
      *     zero means the rest of the line
      * @param title
      *     the column title to use.  Give NULL pointer for none.
      *
      * @returns
      *      output::pointer ; a pointer to an output stream to be used to write
      *      into this column.  Use the delete operator when you are done
      *      with the column.
      */
    virtual output::pointer create(int left, int right, const char *title)
	= 0;

    /**
      * The title method is used to set the two lines of page
      * headings.  These are ignored for unformatted output.
      *
      * @param first
      *     first line of heading, NULL means blank
      * @param second
      *     second line of heading, NULL means blank
      */
    virtual void title(const char *first, const char *second) = 0;

    /**
      * The eoln method is used to signal that an entire line of columns
      * has been assembled and that it should be emitted to the output
      * file or pager.
      *
      * Columns all receive an implicit newline, and are flushed before
      * processing.  All of the columns are thus aligned into rows,
      * even when (due to newlines or wrapping) they would otherwise be
      * different lengths.
      *
      * This is the interface used by clients of this subsystem.
      */
    virtual void eoln() = 0;

    /**
      * The need method is used to nominate that
      * a number of lines is needed before the end of the page.
      * If the lines are available, a blank line is emitted,
      * otherwise a new page is thrown.
      *
      * @param n
      *     the number of lines, not counting the blank line.
      */
    virtual void need(int n) = 0;

    /**
      * The eject method is used to move to the top of the next page,
      * if we aren't there already.
      */
    virtual void eject() = 0;

    /**
      * The flush method is used to force output of all buffered data.
      */
    virtual void flush() = 0;

    /**
      * The forget method is used to discontinue an output column.
      *
      * @param op
      *     The column to forget.
      */
    virtual void forget(const output::pointer &op) = 0;

private:
    /**
      * The copy constructor.  Do not use.
      */
    col(const col &);

    /**
      * The assignment operator.  Do not use.
      */
    col &operator=(const col &);
};

#endif // COL_H
