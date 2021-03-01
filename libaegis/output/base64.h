//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_BASE64_H
#define LIBAEGIS_OUTPUT_BASE64_H

#include <libaegis/output.h>

/**
  * The output_base64 class is used to represent the processing
  * necessary to MIME BASE64 encode an output stream.
  */
class output_base64:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_base64();

private:
    /**
      * The constructor.  It's private on purpose, use the "create"
      * class method instead.
      *
      * \param deeper
      *     The deeper output, the place this filter writes the filtered
      *     output.
      * \param delete_deeper
      *     Wherther or not to delete the deeper output in this
      *     instance's destructor.
      */
    output_base64(const output::pointer &deeper);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * \param deeper
      *     The deeper output, the place this filter writes the filtered
      *     output.
      * \param delete_deeper
      *     Wherther or not to delete the deeper output in this
      *     instance's destructor.
      */
    static pointer create(const output::pointer &deeper);

protected:
    // See base class for documentation.
    nstring filename() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner();

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output to which this filter writes its filtered output.
      */
    output::pointer deeper;

    /**
      * The residual_value instance variable is used to remember the
      * as-yet-unwritten bits of the value.
      */
    unsigned int residual_value;

    /**
      * The residual_bits instance variable is used to remember the
      * number of bits held by the residual_value instance variable.
      */
    int residual_bits;

    /**
      * The output_column instance variable is used to remember what
      * column the output is up to.
      */
    int output_column;

    /**
      * The pos instance variable is used to remember the file position.
      */
    long pos;

    /**
      * The bol instance variable is used to remember whether or not we
      * are positioned at the beginning of a line.
      */
    bool bol;

    /**
      * The default constructor.  Do not use.
      */
    output_base64();

    /**
      * The copy constructor.  Do not use.
      */
    output_base64(const output_base64 &);

    /**
      * The assignment operator.  Do not use.
      */
    output_base64 operator=(const output_base64 &);
};

#endif // LIBAEGIS_OUTPUT_BASE64_H
