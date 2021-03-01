//
// aegis - project change supervisor
// Copyright (C) 2002, 2005, 2006, 2008, 2011, 2012 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_BIT_BUCKET_H
#define LIBAEGIS_OUTPUT_BIT_BUCKET_H

#include <common/nstring.h>
#include <libaegis/output.h>

/**
  * The output_bit_bucket class represents a NULL output stream.  The
  * output written to such a stream will be discarded.
  */
class output_bit_bucket:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_bit_bucket();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method instead.
      */
    output_bit_bucket();

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
    long int ftell_inner(void) const;

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
      * The file_name instance variable is used to remember the name of
      * this "file".
      */
    nstring file_name;

    /**
      * The pos instance variable is used to remember the current output
      * position.
      */
    long pos;

    /**
      * The copy constructor.  Do not use.
      */
    output_bit_bucket(const output_bit_bucket &);

    /**
      * The copy constructor.  Do not use.
      */
    output_bit_bucket &operator=(const output_bit_bucket &);
};

#endif // LIBAEGIS_OUTPUT_BIT_BUCKET_H
// vim: set ts=8 sw=4 et :
