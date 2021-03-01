//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef AEDE_POLICY_VALIDATION_FILES_MERGE_RCS_H
#define AEDE_POLICY_VALIDATION_FILES_MERGE_RCS_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_merge_rcs class is used to represent a test of
  * the files of a change to ensure they do not contain a merge conflict
  * string from the merge(1) tool, from the rcs package.
  */
class validation_files_merge_rcs:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_merge_rcs();

    /**
      * The default constructor.
      */
    validation_files_merge_rcs();

protected:
    // See base class for documentation.
    bool check(change::pointer cp, fstate_src_ty *src);

    // See base class for documentation.
    bool check_binaries() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    validation_files_merge_rcs(const validation_files_merge_rcs &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_merge_rcs &operator=(const validation_files_merge_rcs &);
};

#endif // AEDE_POLICY_VALIDATION_MERGE_RCS_H
