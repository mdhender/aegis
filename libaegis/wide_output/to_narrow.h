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

#ifndef LIBAEGIS_WIDE_OUTPUT_TO_NARROW_H
#define LIBAEGIS_WIDE_OUTPUT_TO_NARROW_H

#include <libaegis/output.h>
#include <libaegis/wide_output.h>

/**
  * The wide_output_to_narrow class represents the filtering necessary
  * to convert a wide output stream into a narrow (multi-byte-char)
  * output stream.
  */
class wide_output_to_narrow:
    public wide_output
{
public:
    /**
      * The destructor.
      */
    virtual ~wide_output_to_narrow();

private:
    /**
      * The constructor.  It is private on purpose, use the #open class
      * method instead.
      *
      * @param deeper
      *     where to write the filtered output
      */
    wide_output_to_narrow(const output::pointer &deeper);

public:
    /**
      * The open class method is used to create new dynamically alocated
      * instances of this class.
      *
      * @param deeper
      *     where to write the filtered output
      */
    static pointer open(const output::pointer &deeper);

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
    output::pointer deeper;
    mbstate_t state;
    bool prev_was_newline;

    /**
      * The default constructor.  Do not use.
      */
    wide_output_to_narrow();

    /**
      * The copy constructor.  Do not use.
      */
    wide_output_to_narrow(const wide_output_to_narrow &);

    /**
      * The assignment operator.  Do not use.
      */
    wide_output_to_narrow &operator=(const wide_output_to_narrow &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_TO_NARROW_H
