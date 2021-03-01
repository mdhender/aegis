//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_INTROSPECTOR_ERROR_H
#define LIBAEGIS_INTROSPECTOR_ERROR_H

#include <libaegis/introspector.h>

/**
  * The introspector_error class is used to represent the processing (or
  * rather lack of processing) required to handle bogus meta-data once
  * errors have been reported.  This silences secondary errors.
  */
class introspector_error:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_error();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      */
    introspector_error();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create();

protected:
    // See base class for documentation.
    void integer(long n);

    // See base class for documentation.
    void real(double n);

    // See base class for documentation.
    void string(const nstring &text);

    // See base class for documentation.
    void enumeration(const nstring &name);

    // See base class for documentation.
    pointer list();

    // See base class for documentation.
    pointer field(const nstring &name);

    // See base class for documentation.
    nstring get_name() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    introspector_error(const introspector_error &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_error &operator=(const introspector_error &);
};

#endif // LIBAEGIS_INTROSPECTOR_ERROR_H
