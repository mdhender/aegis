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
// MANIFEST: interface definition for libaegis/output/memory.c
//

#ifndef LIBAEGIS_OUTPUT_MEMORY_H
#define LIBAEGIS_OUTPUT_MEMORY_H

#include <output.h>

/**
  * The output_memory_ty class is used to represent an output stream
  * which writes to a block of dynamically allocated memory.
  */
class output_memory_ty:
    public output_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~output_memory_ty();

    /**
      * The default constructor.
      */
    output_memory_ty();

    /**
      * The forward method is used to forward the data held by this
      * object to a different output stream.  It does not affect this
      * output stream's data.
      *
      * \note:
      *     The only reason this method isn't const is that we have to
      *     call flush() before we do anything else, to make sure all
      *     the data is in the memory buffer.
      */
    void forward(output_ty *deeper);

    // See base class for documentation.
    string_ty *filename() const;

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    long ftell_inner() const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner();

private:
    /**
      * The buffer instance variable is used to remember the dynamically
      * alloocated buffer to hold the data written to this output
      * stream.
      */
    unsigned char *buffer;

    /**
      * The size instance variable is used to remember how mant
      * charactgers have been saved in the buffer to date.
      */
    size_t size;

    /**
      * The maximum instance variable is used to remember how many
      * characters have been allocated for the buffer.
      */
    size_t maximum;

    /**
      * The copy constructor.  Do not use.
      */
    output_memory_ty(const output_memory_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    output_memory_ty &operator=(const output_memory_ty &);
};

inline DEPRECATED output_ty *
output_memory_open(void)
{
    return new output_memory_ty();
}

inline DEPRECATED void
output_memory_forward(output_memory_ty *fp, output_ty *deeper)
{
    fp->forward(deeper);
}

#endif // LIBAEGIS_OUTPUT_MEMORY_H
