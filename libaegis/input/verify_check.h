//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_VERIFY_CHECK_H
#define LIBAEGIS_INPUT_VERIFY_CHECK_H

#include <libaegis/input.h>

/**
  * The input_verify_checksum class is used to represent an input filter
  * which verifies the Adler 32 checksum of data that it reads.
  */
class input_verify_checksum:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_verify_checksum();

    /**
      * The constructor.
      *
      * @param deeper
      *     The source input stream that this filter uses to obtain its data.
      * @param checksum
      */
    input_verify_checksum(input &deeper, unsigned long checksum);

    // See base class for documentation.
    long read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    long length();

    // See base class for documentation.
    long ftell_inner();

    // See base class for documentation.
    bool is_remote() const;

private:
    /**
      * The deeper instance variable is sued to remember the source
      * input stream that this filter uses to obtain its data.
      */
    input deeper;

    /**
      * The checksum_given instance variable is used to remember the
      * checksum given to the constructor.
      */
    unsigned long checksum_given;

    /**
      * The checksum_calculated instance variable is used to remember
      * the running checksum calculated from the data.
      */
    unsigned long checksum_calculated;

    /**
      * The pos instance variable is used to remember the current file
      * position.
      */
    long pos;

    /**
      * The default constructor.  Do not use.
      */
    input_verify_checksum();

    /**
      * The copy constructor.  Do not use.
      */
    input_verify_checksum(const input_verify_checksum &);

    /**
      * The assignment operator.  Do not use.
      */
    input_verify_checksum &operator=(const input_verify_checksum &);
};

#endif // LIBAEGIS_INPUT_VERIFY_CHECK_H
