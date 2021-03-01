//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
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
// MANIFEST: interface of the validation_crlf class
//

#ifndef AEDE_POLICY_VALIDATION_CRLF_H
#define AEDE_POLICY_VALIDATION_CRLF_H

#include <aede-policy/validation.h>

/**
  * The validation_crlf class is used to represent a policy that does
  * not allow CRLF line termination of text files.
  */
class validation_crlf:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_crlf();

    /**
      * The default constructor.
      */
    validation_crlf();

    // See base class for documentation.
    bool run(change_ty *cp);

private:
    /**
      * The check method is used to check a single file.
      *
      * @param cp
      *     The change in question.
      * @pram src
      *     The file in question.
      * @returns
      *     bool; true if OK, false is a problem is found.
      */
    bool check(change_ty *cp, fstate_src_ty *src) const;

    /**
      * The copy constructor.  Do not use.
      */
    validation_crlf(const validation_crlf &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_crlf &operator=(const validation_crlf &);
};

#endif // AEDE_POLICY_VALIDATION_CRLF_H
