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

#ifndef AEMAKEGEN_TARGET_MAKE_H
#define AEMAKEGEN_TARGET_MAKE_H

#include <aemakegen/target.h>

/**
  * The target_make class is used to represent the processing common to
  * Makefile.in and Makefile.am rather than have method migration to the
  * target base class.
  */
class target_make:
    public target
{
public:
    /**
      * The destructor.
      */
    virtual ~target_make();

protected:
    /**
      * The constructor.
      * For use by derived classes only.
      *
      * @param cid
      *     The location of the change identification
      */
    target_make(change_identifier &cid);

    // See base class for documentation.
    void process_item_aegis_fmtgen(const nstring &filename);

    // See base class for documentation.
    void process_item_configure_ac_in(const nstring &filename);

    // See base class for documentation.
    void process_item_autoconf(const nstring &filename);

private:
    /**
      * The default constructor.  Do not use.
      */
    target_make();

    /**
      * The copy constructor.  Do not use.
      */
    target_make(const target_make &);

    /**
      * The assignment operator.  Do not use.
      */
    target_make &operator=(const target_make &);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_TARGET_MAKE_H
