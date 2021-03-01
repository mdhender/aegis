//
// aegis - project change supervisor
// Copyright (C) 2010, 2012 Peter Miller
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

#ifndef AEMAKEGEN_TARGET_RPM_SPEC_H
#define AEMAKEGEN_TARGET_RPM_SPEC_H

#include <aemakegen/target.h>

/**
  * The target_rpm_spec class is used to represent the processing
  * necessary to produce an RPM .spec file, from the project file
  * manifest.
  */
class target_rpm_spec:
    public target
{
public:
    /**
      * The destructor.
      */
    virtual ~target_rpm_spec();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(change_identifier &cid);

protected:
    // See base class for documentation.
    void process3_begin(void);

    // See base class for documentation.
    void process3_end(void);

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      */
    target_rpm_spec(change_identifier &cid);

#if 0
    bool seen_groff;
    bool seen_tests;
    nstring_list files;
    nstring_list files_libs;
    nstring_list files_devel;
#endif

    /**
      * The copy constructor.  Do not use.
      */
    target_rpm_spec(const target_rpm_spec &);

    /**
      * The assignment operator.  Do not use.
      */
    target_rpm_spec &operator=(const target_rpm_spec &);
};

#endif // AEMAKEGEN_TARGET_RPM_SPEC_H
// vim: set ts=8 sw=4 et :
