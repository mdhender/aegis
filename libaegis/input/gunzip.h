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
// MANIFEST: interface definition for libaegis/input/gunzip.c
//

#ifndef LIBAEGIS_INPUT_GUNZIP_H
#define LIBAEGIS_INPUT_GUNZIP_H

#include <ac/zlib.h>

#include <input.h>

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
      * @param close_on_close
      *     If true, delete the deeper input in the destructor.
      */
    input_gunzip(input_ty *deeper, bool close_on_close = true);

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
    static bool candidate(input_ty *deeper);

private:
    input_ty *deeper;
    bool close_on_close;
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

input_ty *input_gunzip_open(input_ty *ip);

#endif // LIBAEGIS_INPUT_GUNZIP_H
