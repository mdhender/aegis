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

#ifndef LIBAEGIS_OUTPUT_TO_WIDE_H
#define LIBAEGIS_OUTPUT_TO_WIDE_H

#include <libaegis/output.h>
#include <libaegis/wide_output.h>

/**
  * The output_to_wide class is used to represent a narrow character
  * to wide character filter.
  */
class output_to_wide:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_to_wide();

private:
    /**
      * The constructor.  It is private on purpose, use the "open" class
      * method instead.
      *
      * @param deeper
      *     the deeper wide output stream on which this filter writes to.
      */
    output_to_wide(const wide_output::pointer &deeper);

public:
    /**
      * The open class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     the deeper wide output stream on which this filter writes to.
      */
    static pointer open(const wide_output::pointer &deeper);

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
    void flush_inner();

    // See base class for documentation.
    int page_width() const;

    // See base class for documentation.
    int page_length() const;

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * output stream on which this filter writes to.
      */
    wide_output::pointer deeper;

    char *input_buf;
    size_t input_len;
    size_t input_max;
    mbstate_t input_state;
    bool input_bol;
    wchar_t *output_buf;
    size_t output_len;
    size_t output_max;

    /**
      * The default constructor.  Do not use.
      */
    output_to_wide();

    /**
      * The copy constructor.  Do not use.
      */
    output_to_wide(const output_to_wide &);

    /**
      * The assignment operator.  Do not use.
      */
    output_to_wide &operator=(const output_to_wide &);
};

#endif // LIBAEGIS_OUTPUT_TO_WIDE_H
