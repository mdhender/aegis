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

#ifndef LIBAEGIS_OUTPUT_FILTER_WRAP_MAKE_H
#define LIBAEGIS_OUTPUT_FILTER_WRAP_MAKE_H

#include <common/nstring/accumulator.h>
#include <common/nstring/list.h>

#include <libaegis/output/filter.h>

/**
  * The output_filter_wrap_make class is used to represent the
  * processing required to wrap long lines in a Makefile.
  */
class output_filter_wrap_make:
    public output_filter
{
public:
    typedef aegis_shared_ptr<output_filter_wrap_make> pointer;

    /**
      * The destructor.
      */
    virtual ~output_filter_wrap_make();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     Where to write the filtered output.
      */
    static pointer create(const output::pointer &deeper);

protected:
    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param deeper
      *     Where to write the filtered output.
      */
    output_filter_wrap_make(const output::pointer &deeper);

    int column;

    bool tab;

    nstring_accumulator word;

    nstring_list line;

    void write_out_the_line(void);

    /**
      * The default constructor.  Do not use.
      */
    output_filter_wrap_make();

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_wrap_make(const output_filter_wrap_make &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter_wrap_make &operator=(const output_filter_wrap_make &);
};

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_OUTPUT_FILTER_WRAP_MAKE_H
