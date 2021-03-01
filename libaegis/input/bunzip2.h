//
//      aegis - project change supervisor
//      Copyright (C) 2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_INPUT_BUNZIP2_H
#define LIBAEGIS_INPUT_BUNZIP2_H

#include <common/ac/bzlib.h>

#include <libaegis/input.h>

/**
  * The input_bunzip2 class is used to represent an input stream which
  * is uncompressed on the fly.
  */
class input_bunzip2:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_bunzip2();

    /**
      * The constructor.
      *
      * @param deeper
      *     The deeper input which this filter reads from.
      */
    input_bunzip2(input &deeper);

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

    /**
      * The candidate class method is used to check the magic number of a
      * gzipped file.  All of the bytes read are unread before this method
      * returns.
      */
    static bool candidate(input &deeper);

private:
    /**
      * The deeper instance variable is used to remember the deeper
      * input which this filter reads from.
      */
    input deeper;

    enum { BUFFER_SIZE = 1 << 14 };

    bz_stream stream;
    bool end_of_file;
    off_t pos;
    char *buf;
    nstring filename;

    /**
      * The bzlib_fatal_error method is used to report fatal error from
      * the bunzip2 engine.
      *
      * @param err
      *     The error code returned by a deeper bzlib function.
      * @note
      *     This method does not return.
      */
    void bzlib_fatal_error(int err);

    /**
      * The default constructor.
      */
    input_bunzip2();

    /**
      * The copy constructor.
      */
    input_bunzip2(const input_bunzip2 &arg);

    /**
      * The assignment operator.
      */
    input_bunzip2 &operator=(const input_bunzip2 &arg);
};

input input_bunzip2_open(input &ip);

#endif // LIBAEGIS_INPUT_BUNZIP2_H
// vim: set ts=8 sw=4 et :
