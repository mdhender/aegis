//
// aegis - project change supervisor
// Copyright (C) 2002, 2005, 2006, 2008, 2011, 2012 Peter Miller
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

#ifndef AETAR_OUTPUT_TAR_H
#define AETAR_OUTPUT_TAR_H

#include <libaegis/output.h>

/**
  * The output_tar class is used to represent a tar archive.
  */
class output_tar:
    public output
{
public:
    /**
      * The destructor.
      */
    virtual ~output_tar();

    /**
      * The constructor.
      *
      * \param deeper
      *     The underlying output to which the tar archive is to be
      *     written.
      */
    output_tar(const output::pointer &deeper);

    /**
      * The child method is used to create a new output object into
      * which the archive member contents are written.
      *
      * \param name
      *     The name of the archive member.
      * \param len
      *     The exact length of the archive member.
      *     It is a bug if the wrong amount of data is written.
      * \param executable
      *     Wherther ot not the archive member is executable.
      */
    output::pointer child(const nstring &name, long len, bool executable);

protected:
    // See base class for documentation.
    nstring filename(void) const ;

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
      * output to which the tar archive is to be written.
      */
    output::pointer deeper;

    /**
      * The default constructor.  Do not use.
      */
    output_tar();

    /**
      * The copy constructor.  Do not use.
      */
    output_tar(const output_tar &);

    /**
      * The assignment operator.  Do not use.
      */
    output_tar &operator=(const output_tar &);
};

#endif // AETAR_OUTPUT_TAR_H
// vim: set ts=8 sw=4 et :
