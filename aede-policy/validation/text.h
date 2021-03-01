//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
//	All rights reserved.
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
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: interface of the validation_text class
//

#ifndef AEDE_POLICY_VALIDATION_TEXT_H
#define AEDE_POLICY_VALIDATION_TEXT_H

#include <aede-policy/validation.h>

struct fstate_src_ty; // forward

/**
  * The validation_text class is used to represent a check that no file
  * in the change set are binary.  In these days of international text,
  * the only invalid character is NUL, so this is a test for no NUL
  * characters in source files.
  *
  * Note that Aegis doesn't care if your files are binary.  This is
  * just a policy you may want to impose on your project.  If you have
  * configured the history_content_limitation field in the project
  * configuration file properly, neither will your history tool.
  */
class validation_text:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_text();

    /**
      * The default constructor.
      */
    validation_text();

    // See base class for documentation.
    bool run(change_ty *cp);

private:
    /**
      * The check method is used to check a single file.
      *
      * @param cp
      *     The change in question.
      * @param src
      *     The file in question.
      */
    bool check(change_ty *cp, fstate_src_ty *src) const;

    /**
      * The copy constructor.  Do not use.
      */
    validation_text(const validation_text &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_text &operator=(const validation_text &);
};

#endif // AEDE_POLICY_VALIDATION_TEXT_H
