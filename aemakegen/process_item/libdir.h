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

#ifndef AEMAKEGEN_PROCESS_ITEM_LIBDIR_H
#define AEMAKEGEN_PROCESS_ITEM_LIBDIR_H

#include <aemakegen/process_item.h>

/**
  * The process_item_libdir class is used to represent
  * the processing required for "libdir/" files.
  */
class process_item_libdir:
    public process_item
{
public:
    typedef aegis_shared_ptr<process_item_libdir> pointer;

    /**
      * The destructor.
      */
    virtual ~process_item_libdir();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param filename
      *     The name of the file of interest.
      */
    static pointer create(target &tgt);

protected:
    // See base class for documentation.
    bool condition(const nstring &filename);

    // See base class for documentation.
    void preprocess(const nstring &filename);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param filename
      *     The name of the file of interest.
      */
    process_item_libdir(target &tgt);

    /**
      * The default constructor.
      * Do not use.
      */
    process_item_libdir();

    /**
      * The copy constructor.
      * Do not use.
      *
      * @param rhs
      *     The right hand side of the initialization.
      */
    process_item_libdir(const process_item_libdir &rhs);

    /**
      * The assignment operator.
      * Do not use.
      *
      * @param rhs
      *     The right hand side of the assignment.
      */
    process_item_libdir &operator=(const process_item_libdir &rhs);
};

// vim: set ts=8 sw=4 et :
#endif // AEMAKEGEN_PROCESS_ITEM_LIBDIR_H
