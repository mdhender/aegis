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

#ifndef AEDE_POLICY_VALIDATION_FILES_MERGE_FHIST_H
#define AEDE_POLICY_VALIDATION_FILES_MERGE_FHIST_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_merge_fhist class is used to represent a test
  * of the files of a change to ensure they do not contain a merge
  * conflict string from the fmerge(1) tool, from the fhist package.
  */
class validation_files_merge_fhist:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_merge_fhist();

    /**
      * The default constructor.
      */
    validation_files_merge_fhist();

protected:
    // See base class for documentation.
    bool check(change::pointer cp, fstate_src_ty *src);

    // See base class for documentation.
    bool check_binaries() const;

private:
    /**
      * The merge_fhist constructor.  Do not use.
      */
    validation_files_merge_fhist(const validation_files_merge_fhist &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_merge_fhist &operator=(
        const validation_files_merge_fhist &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_MERGE_FHIST_H
