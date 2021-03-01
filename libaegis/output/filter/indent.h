//
// aegis - project change supervisor
// Copyright (C) 1999, 2002, 2005, 2006, 2008, 2011 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_FILTER_INDENT_H
#define LIBAEGIS_OUTPUT_FILTER_INDENT_H

#include <libaegis/output/filter.h>

/**
  * The output_filter_indent class is used to represent an output stream
  * which automatically indents its C-like input.
  */
class output_filter_indent:
    public output_filter
{
public:
    typedef aegis_shared_ptr<output_filter_indent> ipointer;

    /**
      * The destructor.
      */
    virtual ~output_filter_indent();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     the deeper output stream on which this filter writes to.
      */
    static ipointer create(const output::pointer &deeper);

    /**
      * The indent_more method is used to increase the indenting
      * beyond the automatically calculated indent.
      *
      * @note
      *     There must be a matching indent_less call.
      */
    void indent_more(void);

    /**
      * The indent_less function is used to decrease the indenting
      * to less than the automatically calculated indent.
      *
      * @note
      *     There must be a matching indent_more call.
      */
    void indent_less(void);

protected:
    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param deeper
      *     the deeper output stream on which this filter writes to.
      */
    output_filter_indent(const output::pointer &deeper);

    enum {
        /**
          * This defines the width of a tab on output.
          */
        INDENT = 8
    };

    /**
      * The depth instance variable is used to remember the current
      * level of indenting.  Will never be negative.
      */
    int depth;

    /**
      * The in_col instance variable is used to remember the current
      * input column.  This can differe from the output column when
      * white space is being optimized.
      */
    int in_col;

    /**
      * The out_col instance variable is used to remember the current
      * output column.
      */
    int out_col;

    /**
      * The continuation_line instance variable is used to remember
      * whether or not the current line is a continuation line.
      *
      * State 0 means that it is not, state 1 means that a backslash (\)
      * has been seen, and state 2 means that "\\\n" has been seen.
      */
    int continuation_line;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * The default constructor.  Do not use.
      */
    output_filter_indent();

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_indent(const output_filter_indent &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter_indent &operator=(const output_filter_indent &);
};

#endif // LIBAEGIS_OUTPUT_FILTER_INDENT_H
