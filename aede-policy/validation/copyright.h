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
// MANIFEST: interface of the validation_copyright class
//

#ifndef AEDE_POLICY_VALIDATION_COPYRIGHT_H
#define AEDE_POLICY_VALIDATION_COPYRIGHT_H

#include <common/nstring.h>
#include <aede-policy/validation.h>

struct fstate_src_ty; // forward

/**
  * The validation_copyright class is used to represent a test of the
  * files of a change to ensure they all contain an up-to-date copyright
  * notice.
  */
class validation_copyright:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_copyright();

    /**
      * The default constructor.
      */
    validation_copyright();

    // See base class for documentation.
    bool run(change_ty *cp);

private:
    nstring year;
    nstring suggest;
    nstring who;

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
    validation_copyright(const validation_copyright &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_copyright &operator=(const validation_copyright &);
};

#endif // AEDE_POLICY_VALIDATION_COPYRIGHT_H
