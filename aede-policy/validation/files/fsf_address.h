//
// aegis - project change supervisor
// Copyright (C) 2007, 2008, 2010, 2012 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_FILES_FSF_ADDRESS_H
#define AEDE_POLICY_VALIDATION_FILES_FSF_ADDRESS_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_fsf_address class is used to represent a check
  * of source files which looks for the old FSF addresses, and warns
  * about what the new address should be.
  */
class validation_files_fsf_address:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_fsf_address();

private:
    /**
      * The default constructor.
      */
    validation_files_fsf_address();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(void);

protected:
    // See base class for documentation.
    bool check(change::pointer cp, fstate_src_ty *src);

    // See base class for documentation.
    bool check_binaries(void) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    validation_files_fsf_address(const validation_files_fsf_address &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_fsf_address &operator=(
        const validation_files_fsf_address &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_FSF_ADDRESS_H
// vim: set ts=8 sw=4 et :
