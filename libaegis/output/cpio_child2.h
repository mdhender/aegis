//
// aegis - project change supervisor
// Copyright (C) 1999, 2002, 2005, 2006, 2008, 2011, 2012 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_CPIO_CHILD2_H
#define LIBAEGIS_OUTPUT_CPIO_CHILD2_H

#include <common/ac/time.h>

#include <libaegis/output/memory.h>

class output_memory_ty; // forward

/**
  * The output_cpio_child2 class is used to represent a CPIO archive
  * member of unknown length.  It will be cached in memory until the
  * length is known.
  */
class output_cpio_child2:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_cpio_child2();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param deeper
      *     The the underlying output to which the CPIO archive is to be
      *     written.
      * @param name
      *     The name of the archive member.
      * @param mtime
      *     The time to insert into meta data.
      */
    output_cpio_child2(const output::pointer &deeper, const nstring &name,
        time_t mtime);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param deeper
      *     The the underlying output to which the CPIO archive is to be
      *     written.
      * @param name
      *     The name of the archive member.
      * @param mtime
      *     The time to insert into the meta data.
      */
    static pointer create(const output::pointer &deeper, const nstring &name,
        time_t mtime);

protected:
    // See base class for documentation.
    nstring filename(void) const;

    // See base class for documentation.
    nstring type_name(void) const;

    // See base class for documentation.
    long ftell_inner(void) const;

    // See base class for documentation.
    void write_inner(const void *data, size_t length);

    // See base class for documentation.
    void end_of_line_inner(void);

    // See base class for documentation.
    void flush_inner(void);

private:
    /**
      * The deeper instance variable is used to remember the underlying
      * output to which the CPIO archive is to be written.
      */
    output::pointer deeper;

    /**
      * The name instance variable is used to remember the name of the
      * archive member.
      */
    nstring name;

    /**
      * The buffer instance variable is used to remember the data
      * written to the archive member, so that we can obtain its length.
      */
    output_memory::mpointer buffer;

    /**
      * The mtime instance variable is used to remember the time stamp
      * to attach to the file in the archive.
      */
    time_t mtime;

    /**
      * The default constructor.  Do not use.
      */
    output_cpio_child2();

    /**
      * The copy constructor.  Do not use.
      */
    output_cpio_child2(const output_cpio_child2 &);

    /**
      * The assignment operator.  Do not use.
      */
    output_cpio_child2 &operator=(const output_cpio_child2 &);
};

#endif // LIBAEGIS_OUTPUT_CPIO_CHILD2_H
// vim: set ts=8 sw=4 et :
