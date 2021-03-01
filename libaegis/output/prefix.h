//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_PREFIX_H
#define LIBAEGIS_OUTPUT_PREFIX_H

#include <common/nstring.h>
#include <libaegis/output.h>


/**
  * The output_prefix class is used to represent an output stream
  * filter which adds a prefix to every line of output.
  */
class output_prefix:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_prefix();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * \param deeper
      *     the deeper output stream on which this filter writes to.
      * \param prefix
      *     the prefix to be added to each line.
      */
    output_prefix(const output::pointer &deeper, const char *prefix);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * \param deeper
      *     the deeper output stream on which this filter writes to.
      * \param prefix
      *     the prefix to be added to each line.
      */
    static pointer create(const output::pointer &deeper, const char *prefix);

protected:
    // See base class for documentation.
    nstring filename() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    void flush_inner();

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output stream on which this filter writes to.
      */
    output::pointer deeper;

    /**
      * The prefix instance variable is used to remember the prefix to
      * be added to each line.
      */
    nstring prefix;

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
      * The default constructor.  Do not use.
      */
    output_prefix();

    /**
      * The copy constructor.  Do not use.
      */
    output_prefix(const output_prefix &);

    /**
      * The assignment operator.  Do not use.
      */
    output_prefix &operator=(const output_prefix &);
};

#endif // LIBAEGIS_OUTPUT_PREFIX_H
