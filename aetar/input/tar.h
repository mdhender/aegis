//
//      aegis - project change supervisor
//      Copyright (C) 2002, 2005, 2006, 2008, 2012 Peter Miller
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

#ifndef AETAR_INPUT_TAR_H
#define AETAR_INPUT_TAR_H

#include <libaegis/input.h>

/**
  * The input_tar class is used to represent a tar archive.
  */
class input_tar:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_tar();

    /**
      * The constructor.
      *
      * @param deeper
      *     the data sourec for this filter.
      */
    input_tar(input &deeper);

    /**
      * The child method is used to obtain the next archive element.
      *
      * @param archive_name
      *     Use to return the name of the file in the archive to the caller.
      * @param executable
      *     Use to return the executability of the file to the caller.
      * @returns
      *     NULL at end of file, or a valid input instance to read the
      *     file contents the file contents from.
      */
    input child(nstring &archive_name, bool &executable);

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    off_t length();

    // See base class for documentation.
    ssize_t read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    off_t ftell_inner();

private:
    /**
      * The depeer instance variable is used to remember the data source
      * for this filter.
      */
    input deeper;

    nstring read_data_as_string(size_t hsize);
    void padding();

    /**
      * The default constructor.
      */
    input_tar();

    /**
      * The copy constructor.
      */
    input_tar(const input_tar &arg);

    /**
      * The assignment operator.
      */
    input_tar &operator=(const input_tar &arg);
};

#endif // AETAR_INPUT_TAR_H
// vim: set ts=8 sw=4 et :
