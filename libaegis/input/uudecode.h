//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2005, 2006 Peter Miller
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
// MANIFEST: interface definition for libaegis/input/uudecode.c
//

#ifndef LIBAEGIS_INPUT_UUDECODE_H
#define LIBAEGIS_INPUT_UUDECODE_H

#include <common/ac/limits.h>

#include <libaegis/input.h>

/**
  * The input_uudecode class represents an input source which is
  * transparently uudecoded.
  */
class input_uudecode:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_uudecode();

    /**
      * The constructor.
      *
      * @param deeper
      *     The source of data for this filter.
      */
    input_uudecode(input &deeper);

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
      * The candidate class method is used to determine whether or not
      * an input needs to be uudecoded.  All input is pushed back, no
      * matter what the result is.
      */
    static bool candidate(input &deeper);

private:
    /**
      * The deeper instance variable is used to remember the source of
      * data for this filter.
      */
    input deeper;

    long pos;
    int state;
    char etab[64];
    unsigned char itab[UCHAR_MAX + 1];
    int checksum;
    int padding;

    /**
      * The default constructor.  Do not use.
      */
    input_uudecode();

    /**
      * The copy constructor.  Do not use.
      */
    input_uudecode(const input_uudecode &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_uudecode &operator=(const input_uudecode &arg);
};

DEPRECATED inline bool
input_uudecode_recognise(input ip)
{
    return input_uudecode::candidate(ip);
}

#endif // LIBAEGIS_INPUT_UUDECODE_H
