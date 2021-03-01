//
//	aegis - project change supervisor
//	Copyright (C) 2006-2008 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_FILES_PRINTABLE_H
#define AEDE_POLICY_VALIDATION_FILES_PRINTABLE_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_printable class is used to represent a check
  * that all files in the change set contain only printable ACII
  * characters and white space.  This includes being 7-bit clean.
  *
  * Note that Aegis doesn't care if your files are unprintable.  This
  * is just a policy you may want to impose on your project.  If you
  * have configured the history_content_limitation field in the project
  * configuration file properly, your history tool will not care if
  * files are unprintable, either.
  */
class validation_files_printable:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_printable();

    /**
      * The default constructor.
      */
    validation_files_printable();

protected:
    // See base class for documentation.
    bool check(change::pointer cp, fstate_src_ty *src);

    // See base class for documentation.
    bool check_binaries() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    validation_files_printable(const validation_files_printable &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_printable &operator=(const validation_files_printable &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_PRINTABLE_H
