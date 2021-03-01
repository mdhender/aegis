//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_FILTER_WRAP_SIMPLE_H
#define LIBAEGIS_OUTPUT_FILTER_WRAP_SIMPLE_H

#include <common/nstring/accumulator.h>

#include <libaegis/output/filter.h>

/**
  * The output_filter_wrap_simple class is used to represent simple
  * wrapping of simple ASCII text, with no provision for breaking at any
  * point except for at white space.
  *
  * If you need to handle multi-byte encodings, use the
  * #output_wrap_open function instead.
  */
class output_filter_wrap_simple:
    public output_filter
{
public:
    typedef aegis_shared_ptr<output_filter_wrap_simple> pointer;

    /**
      * The destructor.
      */
    virtual ~output_filter_wrap_simple();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     where to write the filtered output
      */
    static pointer create(const output::pointer &deeper);

protected:
    // See base class for documentation.
    void write_inner(const void *data, size_t data_size);

    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    void end_of_line_inner(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param deeper
      *     where to write the filtered output
      */
    output_filter_wrap_simple(const output::pointer &deeper);

    /**
      * The word instance variable is used to remember the word
      * currently being accumulated.
      */
    nstring_accumulator word;

    /**
      * The icol instance variable is used to remember the current
      * input column.
      */
    int icol;

    /**
      * The ocol instance variable is used to remember the current
      * output column.
      */
    int ocol;

    /**
      * The default constructor.  Do not use.
      */
    output_filter_wrap_simple();

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_wrap_simple(const output_filter_wrap_simple &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter_wrap_simple &operator=(const output_filter_wrap_simple &);
};

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_OUTPUT_FILTER_WRAP_SIMPLE_H
