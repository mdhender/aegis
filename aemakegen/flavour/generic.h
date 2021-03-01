//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#ifndef AEMAKEGEN_FLAVOUR_GENERIC_H
#define AEMAKEGEN_FLAVOUR_GENERIC_H

#include <aemakegen/flavour.h>

/**
  * The flavour_generic class is used to represent process injection for
  * generic projects.
  */
class flavour_generic:
    public flavour
{
public:
    /**
      * The destructor.
      */
    virtual ~flavour_generic();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param tgt
      *     the target instance we are bound to.
      */
    static pointer create(target &tgt);

protected:
    // See base class for documentation.
    void set_process(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param tgt
      *     the target instance we are bound to.
      */
    flavour_generic(target &tgt);

    target &tgt2;

    /**
      * The default constructor.
      * Do not use.
      */
    flavour_generic();

    /**
      * The copy constructor.  Do not use.
      */
    flavour_generic(const flavour_generic &);

    /**
      * The assignment operator.  Do not use.
      */
    flavour_generic &operator=(const flavour_generic &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_FLAVOUR_GENERIC_H
