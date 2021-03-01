//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002, 2005, 2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_COL_PRETTY_H
#define LIBAEGIS_COL_PRETTY_H

#include <libaegis/col.h>
#include <libaegis/output.h>
#include <libaegis/wide_output/column.h>

/**
  * The col_pretty class is used to represent multi-column output with
  * line wrap around the column values, and formatting across multiple
  * lines.
  */
class col_pretty:
    public col
{
public:
    /**
      * The destructor.
      */
    virtual ~col_pretty();

private:
    /**
      * The constructor.  Its is private on pupose, use the #create
      * class method instead.
      *
      * @param deeper
      *     The output stream upon which to write the columnar output.
      */
    col_pretty(const wide_output::pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     The output stream upon which to write the columnar output.
      */
    static pointer create(const wide_output::pointer &deeper);

protected:
    // See base class for documentation.
    output::pointer create(int left, int right, const char *title);

    // See base class for documentation.
    void title(const nstring &first, const nstring &second);

    // See base class for documentation.
    void eoln();

    // See base class for documentation.
    void need(int n);

    // See base class for documentation.
    void eject();

    // See base class for documentation.
    void flush();

    // See base class for documentation.
    void forget(const output::pointer &op);

private:
    wide_output::pointer deeper;
    bool need_to_emit_headers;
    size_t ncolumns;
    size_t ncolumns_max;

    struct column_ty
    {
        ~column_ty();
        column_ty();
        column_ty(const column_ty &);
        column_ty &operator=(const column_ty &);

        wide_output_column::cpointer header;
        wide_output_column::cpointer content;
        output::pointer content_filter;
        int left;
        int right;

        void clear();
    };

    column_ty *column;

    struct emit_ty
    {
        emit_ty() : left(0) { }

        wide_output_column::cpointer content;
        int left;
    };

    /**
      * The emit method is used to emit the given values of text out
      * of the deeper output stream.  The emitting is factored into this
      * common method, rather than reproduce the code twice, once
      * for the headers and again for the content.
      */
    void emit(size_t argc, emit_ty *argv, int minlines, bool is_the_header);

    /**
      * The emit_header method is used to emit column headers when
      * necessary.  It clears the flag which controls emitting the
      * headers, too.
      */
    void emit_header();

    /**
      * The emit_content method is used to generate the output, given
      * the content written and formatted via the wide output streams
      * privide by create (below).
      *
      * Usually, the column headers are handled by the top-of-page
      * callback, however if new columns with headers are created in the
      * middle of the page, this method also causes the new column
      * headings to be emitted, before the content.
      */
    void emit_content();

    /**
      * The default constructor.  Do not use.
      */
    col_pretty();

    /**
      * The copy constructor.  Do not use.
      */
    col_pretty(const col_pretty &);

    /**
      * The assignment operator.  Do not use.
      */
    col_pretty &operator=(const col_pretty &);
};

#endif // LIBAEGIS_COL_PRETTY_H
// vim: set ts=8 sw=4 et :
