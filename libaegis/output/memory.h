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

#ifndef LIBAEGIS_OUTPUT_MEMORY_H
#define LIBAEGIS_OUTPUT_MEMORY_H

#include <common/nstring/accumulator.h>
#include <libaegis/output.h>

/**
  * The output_memory class is used to represent an output stream
  * which writes to a block of dynamically allocated memory.
  */
class output_memory:
    public output
{
public:
    typedef aegis_shared_ptr<output_memory> mpointer;

    /**
      * The destructor.
      */
    virtual ~output_memory();

private:
    /**
      * The default constructor.  It is private on purpose, use the
      * "create" class method instead.
      */
    output_memory();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static mpointer create();

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
    void forward(output::pointer deeper);

    /**
      * The mkstr method is used to turn the accumulated data into a
      * string.
      *
      * \note:
      *     The only reason this method isn't const is that we have to
      *     call flush() before we do anything else, to make sure all
      *     the data is in the memory buffer.
      */
    nstring mkstr();

protected:
    // See base class for documentation.
    nstring filename() const;

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
    nstring_accumulator buffer;

    /**
      * The copy constructor.  Do not use.
      */
    output_memory(const output_memory &);

    /**
      * The assignment operator.  Do not use.
      */
    output_memory &operator=(const output_memory &);
};

#endif // LIBAEGIS_OUTPUT_MEMORY_H
