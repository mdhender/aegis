//
//	aegis - project change supervisor
//	Copyright (C) 2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
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

#ifndef AEDE_POLICY_VALIDATION_FILES_WHITE_SPACE_H
#define AEDE_POLICY_VALIDATION_FILES_WHITE_SPACE_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_white_space class is used to represent checking
  * for white space on the ends of lines, and blank lines on the end of
  * files.
  */
class validation_files_white_space:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_white_space();

    /**
      * The default constructor.
      */
    validation_files_white_space();

protected:
    // See base class for documentation.
    bool check(change::pointer cp, fstate_src_ty *src);

    // See base class for documentation.
    bool check_binaries() const;

    // See base class for documentation.
    bool check_downloaded() const;

    // See base class for documentation.
    bool check_foreign_copyright() const;

    // See base class for documentation.
    bool check_branches() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    validation_files_white_space(const validation_files_white_space &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_white_space &operator=(
        const validation_files_white_space &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_WHITE_SPACE_H
