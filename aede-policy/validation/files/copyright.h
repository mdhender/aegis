//
// aegis - project change supervisor
// Copyright (C) 2005-2008, 2010, 2012 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_FILES_COPYRIGHT_H
#define AEDE_POLICY_VALIDATION_FILES_COPYRIGHT_H

#include <common/nstring.h>

#include <aede-policy/validation/files.h>

/**
  * The validation_files_copyright class is used to represent a test
  * of the files of a change to ensure they all contain an up-to-date
  * copyright notice.
  */
class validation_files_copyright:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_copyright();

private:
    /**
      * The default constructor.
      */
    validation_files_copyright();

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
    bool check_branches(void) const;

    // See base class for documentation.
    bool check_downloaded(void) const;

    // See base class for documentation.
    bool check_foreign_copyright(void) const;

    // See base class for documentation.
    bool check_binaries(void) const;

private:
    nstring year;
    nstring suggest;
    nstring who;

    /**
      * The copy constructor.  Do not use.
      */
    validation_files_copyright(const validation_files_copyright &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_copyright &operator=(const validation_files_copyright &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_COPYRIGHT_H
// vim: set ts=8 sw=4 et :
