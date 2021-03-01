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

#ifndef AEDIST_OUTPUT_CPIO_H
#define AEDIST_OUTPUT_CPIO_H

#include <common/ac/time.h>

#include <libaegis/output.h>

struct string_ty; // forward

/**
  * The output_cpio class is used to represent a container class which
  * produces cpio(7) fomatted output streams.
  */
class output_cpio:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_cpio();

    /**
      * The constructor.
      *
      * \param deeper
      *     The underlying output to which the CPIO archive is to be
      *     written.
      * \param mtime
      *     The time stamp to attach to all archive members
      */
    output_cpio(const output::pointer &deeper, time_t mtime);

    /**
      * The child method is used to create a child archive.
      *
      * \param name
      *     The name of the file in the archive.
      * \param len
      *     The length of the archive.  Use -1 if you don't know and it
      *     will be stached into memory until the length is known.
      */
    output::pointer child(const nstring &name, long len);

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

private:
    /**
      * The deeper instance variable is used to remember the underlying
      * output to which the CPIO archive is to be written.
      */
    output::pointer deeper;

    /**
      * The mtime instance variable is used to remember the time to
      * attach to all archive members.
      */
    time_t mtime;

    /**
      * The default constructor.
      */
    output_cpio();

    /**
      * The copy constructor.  Do not use.
      */
    output_cpio(const output_cpio &);

    /**
      * The assignment operator.  Do not use.
      */
    output_cpio &operator=(const output_cpio &);
};

#endif // AEDIST_OUTPUT_CPIO_H
// vim: set ts=8 sw=4 et :
