//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2011, 2012 Peter Miller
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

#ifndef AEMAKEGEN_TARGET_MAKE_AUTOMAKE_H
#define AEMAKEGEN_TARGET_MAKE_AUTOMAKE_H

#include <aemakegen/target/make.h>

/**
  * The target_make_automake class is used to represent the code generation
  * needed to build a Makefile.am automake input file from the Aegis
  * file manifest.
  */
class target_make_automake:
    public target_make
{
public:
    /**
      * The destructor.
      */
    virtual ~target_make_automake();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param cid
      *     The location of the change identification
      */
    static pointer create(change_identifier &cid);

protected:
    // See base class for documentation.
    void process3_begin(void);

    // See base class for documentation.
    void process_item_scripts(const nstring &filename);

    // See base class for documentation.
    void process_item_uudecode(const nstring &filename);

    // See base class for documentation.
    void process_item_aegis_lib_doc(const nstring &filename);

    // See base class for documentation.
    void process3_end(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param cid
      *     The location of the change identification
      */
    target_make_automake(change_identifier &cid);

    /**
      * The default constructor.  Do not use.
      */
    target_make_automake();

    /**
      * The copy constructor.  Do not use.
      */
    target_make_automake(const target_make_automake &);

    /**
      * The assignment operator.  Do not use.
      */
    target_make_automake &operator=(const target_make_automake &);
};

#endif // AEMAKEGEN_TARGET_MAKE_AUTOMAKE_H
// vim: set ts=8 sw=4 et :
