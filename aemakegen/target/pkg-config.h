//
// aegis - project change supervisor
// Copyright (C) 2008, 2011, 2012 Peter Miller
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

#ifndef AEMAKEGEN_TARGET_PKG_CONFIG_H
#define AEMAKEGEN_TARGET_PKG_CONFIG_H

#include <aemakegen/target.h>

/**
  * The target_pkg_config class is used to represent the processing
  * necessary to produce a pkg-config .pc file, from the project file
  * manifest.
  */
class target_pkg_config:
    public target
{
public:
    /**
      * The destructor.
      */
    virtual ~target_pkg_config();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(change_identifier &cid);

protected:
    // See base class for documentation.
    void process3_end(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      */
    target_pkg_config(change_identifier &cid);

    /**
      * The default constructor.  Do not use.
      */
    target_pkg_config();

    /**
      * The copy constructor.  Do not use.
      */
    target_pkg_config(const target_pkg_config &);

    /**
      * The assignment operator.  Do not use.
      */
    target_pkg_config &operator=(const target_pkg_config &);
};

#endif // AEMAKEGEN_TARGET_PKG_CONFIG_H
// vim: set ts=8 sw=4 et :
