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

#ifndef LIBAEGIS_INPUT_GUNZIP_H
#define LIBAEGIS_INPUT_GUNZIP_H

#include <common/ac/zlib.h>

#include <libaegis/input.h>

/**
  * The input_gunzip class is used to represent an input stream which is
  * uncompressed on the fly.
  */
class input_gunzip:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_gunzip();

    /**
      * The constructor.
      *
      * @param deeper
      *     The deeper input which this filter reads from.
      */
    input_gunzip(input &deeper);

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

    /**
      * The candidate class method is used to check the magi number of a
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

    z_stream stream;
    bool z_eof;
    uLong crc;
    long pos;
    Byte *buf;
    nstring filename;

    void zlib_fatal_error(int err);
    long getLong();
    void read_header();

    /**
      * The default constructor.
      */
    input_gunzip();

    /**
      * The copy constructor.
      */
    input_gunzip(const input_gunzip &arg);

    /**
      * The assignment operator.
      */
    input_gunzip &operator=(const input_gunzip &arg);
};

input input_gunzip_open(input &ip);

#endif // LIBAEGIS_INPUT_GUNZIP_H
