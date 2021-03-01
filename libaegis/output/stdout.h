//
// aegis - project change supervisor
// Copyright (C) 1999, 2002, 2004-2006, 2008, 2011, 2012 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_STDOUT_H
#define LIBAEGIS_OUTPUT_STDOUT_H

#include <libaegis/output.h>

/**
  * The output_stdout class is used to repersent the state of an
  * output stream being written to the standard output.
  */
class output_stdout:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_stdout();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * #create class method instead.
      */
    output_stdout();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(void);

protected:
    // See base class for documentation.
    nstring filename(void) const;

    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

    // See base class for documentation.
    int page_width(void) const;

    // See base class for documentation.
    int page_length(void) const;

private:
    /**
      * The bol instance variable is used to remember whether or not we
      * are at the beginning of a line.
      */
    bool bol;

    /**
      * The pos instance variable is used to remember our relative
      * position in the standard output stream.
      */
    long pos;

    /**
      * The copy constructor.  Do not use.
      */
    output_stdout(const output_stdout &);

    /**
      * The assignment operator.  Do not use.
      */
    output_stdout &operator=(const output_stdout &);
};

#endif // LIBAEGIS_OUTPUT_STDOUT_H
// vim: set ts=8 sw=4 et :
