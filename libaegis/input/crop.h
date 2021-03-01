//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_CROP_H
#define LIBAEGIS_INPUT_CROP_H

#include <libaegis/input.h>

/**
  * The input_crop class is used to represent an input which will read
  * only a portion of the deeper input, from the current position.
  */
class input_crop:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_crop();

    /**
      * The constructor.
      *
      * @param deeper
      *     the data source for this filter.
      * @param length
      *     How many bytes of data are in the cropped region.
      */
    input_crop(input &deeper, long length);

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

    void set_name(const nstring &arg) { name_cache = arg; }

private:
    /**
      * The deeper instance variable is used to remember the data source
      * for this filter.
      */
    input deeper;

    /**
      * The deeper instance variable is used to remember how many bytes
      * of data are in the cropped region.
      */
    long maximum;

    /**
      * The pos instance variable is used to remember where we are up to
      * in the crop region.
      */
    long pos;

    /**
      * The name_cache instance variable is used to remember the name of
      * the deeper input stream.
      */
    nstring name_cache;

    /**
      * The default constructor.  Do not use.
      */
    input_crop();

    /**
      * The copy constructor.  Do not use.
      */
    input_crop(const input_crop &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_crop &operator=(const input_crop &arg);
};

#endif // LIBAEGIS_INPUT_CROP_H
