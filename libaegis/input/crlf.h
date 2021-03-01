//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2005 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface definition for libaegis/input/crlf.c
//

#ifndef LIBAEGIS_INPUT_CRLF_H
#define LIBAEGIS_INPUT_CRLF_H

#include <input.h>

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

    /**
      * The constructor.
      */
    input_crlf(input_ty *deeper, bool close_on_close);

    /**
      * The escaped_newline method is used to set a mode where \\\n
      * and \\\\r\\n sequenes are removed.
      */
    void escaped_newline();

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    void keepalive();

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    long ftell_inner();

    // See base class for documentation.
    bool is_remote() const;

private:
    input_ty *deeper;
    bool delete_on_close;
    long pos;
    long line_number;
    bool prev_was_newline;
    nstring name_cache;
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
