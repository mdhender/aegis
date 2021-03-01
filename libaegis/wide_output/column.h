//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_WIDE_OUTPUT_COLUMN_H
#define LIBAEGIS_WIDE_OUTPUT_COLUMN_H

#include <libaegis/wide_output.h>

struct column_row_ty
{
    size_t length_max;
    size_t length;
    wchar_t *text;
    int printing_width;
};

/**
  * The wide_output_column class is used to store columns, in
  * preparation for assembing them ready for output.  The number of
  * lines in the column is tracked, as is the printing width of the
  * column.  This simplifies the task of printing the columns out,
  * particularly if a wrapping filter is imposed before this one.
  */
class wide_output_column:
    public wide_output
{
public:
    typedef aegis_shared_ptr<wide_output_column> cpointer;

    /**
      * The destructor.
      */
    virtual ~wide_output_column();

private:
    /**
      * The constructor.  It is private on purpose, use the
      * #create class method instead.
      */
    wide_output_column(int width, int length);

public:
    /**
      * The default constructor.  It is private on purpose, use the
      * #create class method instead.
      */
    static cpointer open(int width, int length);

    column_row_ty *get(int n);

    void clear_buffers();

protected:
    // See base class for documentation.
    nstring filename();

    // See base class for documentation.
    int page_width();

    // See base class for documentation.
    int page_length();

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    void write_inner(const wchar_t *data, size_t len);

    // See base class for documentation.
    void flush_inner();

    // See base class for documentation.
    void end_of_line_inner();

private:
    int width;
    int length;

    column_row_ty *row;
    size_t nrows;
    size_t nrows_max;

    /**
      * The default constructor.  Do not use.
      */
    wide_output_column();

    wide_output_column(const wide_output_column &);

    wide_output_column &operator=(const wide_output_column &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_COLUMN_H
