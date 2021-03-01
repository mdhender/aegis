//
// aegis - project change supervisor
// Copyright (C) 1999, 2002, 2005, 2006, 2008, 2012 Peter Miller
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

#ifndef LIBAEGIS_COL_UNFORMATTED_H
#define LIBAEGIS_COL_UNFORMATTED_H

#include <common/ac/stddef.h>

#include <libaegis/col.h>
#include <libaegis/wide_output/column.h>


/**
  * The col_unformatted class is used to represent columnar output wich
  * simply has the columns concatenated on a single line, without any
  * formatting.
  */
class col_unformatted:
    public col
{
public:
    /**
      * The destructor.
      */
    virtual ~col_unformatted();

private:
    /**
      * The constructor.  It is private on purpose, use the #create
      * class method instead.
      *
      * @param deeper
      *     the output stream upon which to write the columnar output
      */
    col_unformatted(const wide_output::pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     the output stream upon which to write the columnar output
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
    struct column_ty
    {
        ~column_ty();
        column_ty();

        wide_output_column::cpointer content;
        output::pointer content_filter;

        void clear();
    };

    wide_output::pointer deeper;
    wchar_t separator;
    size_t ncolumns;
    size_t ncolumns_max;
    column_ty *column;

    /**
      * The default constructor.  Do not use.
      */
    col_unformatted();

    /**
      * The copy constructor.  Do not use.
      */
    col_unformatted(const col_unformatted &);

    /**
      * The assignment operator.  Do not use.
      */
    col_unformatted &operator=(const col_unformatted &);
};

#endif // LIBAEGIS_COL_UNFORMATTED_H
// vim: set ts=8 sw=4 et :
