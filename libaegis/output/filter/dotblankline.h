//
// aegis - project change supervisor
// Copyright (C) 2008, 2011 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_OUTPUT_FILTER_DOTBLANKLINE_H
#define LIBAEGIS_OUTPUT_FILTER_DOTBLANKLINE_H

#include <libaegis/output/filter.h>

/**
  * The output_filter_dot_blank_lines class is used to represent a
  * filter that replaces blank lines with a single dot.
  */
class output_filter_dot_blank_lines:
    public output_filter
{
public:
    /**
      * The destructor.
      */
    virtual ~output_filter_dot_blank_lines();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(const pointer &deeper);

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
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      */
    output_filter_dot_blank_lines(const pointer &deeper);

    int column;
    long pos;

    /**
      * The default constructor.  Do not use.
      */
    output_filter_dot_blank_lines(void);

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_dot_blank_lines(const output_filter_dot_blank_lines &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter_dot_blank_lines &operator=(
        const output_filter_dot_blank_lines &);
};

#endif // LIBAEGIS_OUTPUT_FILTER_DOTBLANKLINE_H
