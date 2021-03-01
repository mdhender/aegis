//
// aegis - project change supervisor
// Copyright (C) 2009 Peter Miller
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

#ifndef LIBAEGIS_INPUT_MULTIPART_H
#define LIBAEGIS_INPUT_MULTIPART_H

#include <libaegis/input.h>

/**
  * The input_multipart class is used to represent an input that
  * extracts the first part of a MIME multipart message.
  */
class input_multipart:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_multipart();

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create method instead.
      *
      * @param deeper
      *     the data source for this filter.
      * @param boundary
      *     The boundary string separating the parts
      */
    input_multipart(input &deeper, const nstring &boundary);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     the data source for this filter.
      * @param boundary
      *     The boundary string separating the parts
      */
    static input create(input &deeper, const nstring &boundary);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    off_t length();

    // See base class for documentation.
    void keepalive();

    // See base class for documentation.
    ssize_t read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    off_t ftell_inner();

    // See base class for documentation.
    bool is_remote() const;

private:
    /**
      * The deeper instance variable is used to remember the data source
      * for this filter.
      */
    input deeper;

    nstring boundary1;
    nstring boundary2;
    int state;
    unsigned long spos;

    enum line_type_t
    {
        line_type_content,
        line_type_empty,
        line_type_boundary,
        line_type_last_boundary,
    };

    line_type_t line_type(const nstring &line);

    /**
      * The default constructor.  Do not use.
      */
    input_multipart();

    /**
      * The copy constructor.  Do not use.
      */
    input_multipart(const input_multipart &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_multipart &operator=(const input_multipart &arg);
};

#endif // LIBAEGIS_INPUT_MULTIPART_H
