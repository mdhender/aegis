//
// aegis - project change supervisor
// Copyright (C) 1999, 2002, 2005, 2006, 2008, 2010, 2012 Peter Miller
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

#ifndef LIBAEGIS_INPUT_CRLF_H
#define LIBAEGIS_INPUT_CRLF_H

#include <libaegis/input.h>

/**
  * The input_crlf class is used to transparently filter CRLF sequences
  * into NL sequences.
  */
class input_crlf:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_crlf();

private:
    /**
      * The constructor.
      *
      * @param deeper
      *     The data source we are to filter.
      * @param esc_nl
      *     true if we are to operate in a mode where \\\n and \\\\r\\n
      *     sequences are to be removed.
      */
    input_crlf(input &deeper, bool esc_nl);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     The data source we are to filter.
      * @param esc_nl
      *     true if we are to operate in a mode where \\\n and \\\\r\\n
      *     sequences are to be removed.
      */
    static input create(input &deeper, bool esc_nl = false);

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
      * The deeper instance variable is used to remember the deeper
      * input source of this filter.
      */
    input deeper;

    off_t pos;
    long line_number;
    bool prev_was_newline;
    nstring name_cache;

    /**
      * The newlines_may_be_escaped instance variable is used to
      * remember whether we are in the mode where \\\n and \\\\r\\n
      * sequenes are removed.
      */
    bool newlines_may_be_escaped;

    /**
      * The default constructor.  Do not use.
      */
    input_crlf();

    /**
      * The copy constructor.  Do not use.
      */
    input_crlf(const input_crlf &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_crlf &operator=(const input_crlf &arg);
};

#endif // LIBAEGIS_INPUT_CRLF_H
// vim: set ts=8 sw=4 et :
