//
// aegis - project change supervisor
// Copyright (C) 2006-2008, 2010, 2012 Peter Miller
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

#ifndef AEDE_POLICY_VALIDATION_FILES_TEXT_H
#define AEDE_POLICY_VALIDATION_FILES_TEXT_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_text class is used to represent a check that no
  * file in the change set are binary.  In these days of international
  * text, the only invalid character is NUL, so this is a test for no
  * NUL characters in source files.
  *
  * Note that Aegis doesn't care if your files are binary.  This is
  * just a policy you may want to impose on your project.  If you have
  * configured the history_content_limitation field in the project
  * configuration file properly, neither will your history tool.
  */
class validation_files_text:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_text();

private:
    /**
      * The default constructor.
      */
    validation_files_text();

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
    validation_files_text(const validation_files_text &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_text &operator=(const validation_files_text &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_TEXT_H
// vim: set ts=8 sw=4 et :
