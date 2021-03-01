//
// aegis - project change supervisor
// Copyright (C) 2001, 2002, 2005, 2006, 2008, 2011 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_FILTER_PREFIX_H
#define LIBAEGIS_OUTPUT_FILTER_PREFIX_H

#include <libaegis/output/filter.h>


/**
  * The output_filter_prefix class is used to represent an output stream
  * filter which adds a prefix to every line of output.
  */
class output_filter_prefix:
    public output_filter
{
public:
    /**
      * The destructor.
      */
    virtual ~output_filter_prefix();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     the deeper output stream on which this filter writes to.
      * @param prefix
      *     the prefix to be added to each line.
      */
    static pointer create(const output::pointer &deeper, const char *prefix);

protected:
    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    int page_width(void) const;

    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    void end_of_line_inner(void);

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param deeper
      *     the deeper output stream on which this filter writes to.
      * @param prefix
      *     the prefix to be added to each line.
      */
    output_filter_prefix(const output::pointer &deeper, const nstring &prefix);

    /**
      * The prefix instance variable is used to remember the prefix to
      * be added to each line.
      */
    nstring prefix;

    /**
      * The prefix0 instance variable is used to remember the prefix to
      * be added to each line, if the line is empty.
      */
    nstring prefix0;

    /**
      * The prefix instance variable is used to remember the current
      * output column.
      */
    int column;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * The width instance variable is used to remember the avauilable
      * output width, once the prefix has been subtracted.
      */
    int width;

    /**
      * The default constructor.  Do not use.
      */
    output_filter_prefix();

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_prefix(const output_filter_prefix &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter_prefix &operator=(const output_filter_prefix &);
};

#endif // LIBAEGIS_OUTPUT_FILTER_PREFIX_H
