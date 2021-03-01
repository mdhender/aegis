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

#ifndef AEMAKEGEN_FLAVOUR_AEGIS_H
#define AEMAKEGEN_FLAVOUR_AEGIS_H

#include <aemakegen/flavour.h>

/**
  * The flavour_aegis class is used to represent additional processing
  * specific to Aegis' project shape.
  */
class flavour_aegis:
    public flavour
{
public:
    /**
      * The destructor.
      */
    virtual ~flavour_aegis();

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
    flavour_aegis(target &tgt);

    target &tgt2;

    /**
      * The default constructor.
      * Do not use.
      */
    flavour_aegis();

    /**
      * The copy constructor.  Do not use.
      */
    flavour_aegis(const flavour_aegis &);

    /**
      * The assignment operator.  Do not use.
      */
    flavour_aegis &operator=(const flavour_aegis &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_FLAVOUR_AEGIS_H
