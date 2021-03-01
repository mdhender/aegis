//
// aegis - project change supervisor
// Copyright (C) 2009, 2010 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef AEDE_POLICY_VALIDATION_FILES_RESERVED_H
#define AEDE_POLICY_VALIDATION_FILES_RESERVED_H

#include <common/nstring/list.h>

#include <aede-policy/validation/files.h>

/**
  * The validation_files_reserved class is used to represent checking
  * for C++ reserved words in C++ files, and C reserved words in C
  * files.  (ANSI C and C++ standards, section 10.2 in both.)
  */
class validation_files_reserved:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_reserved();

private:
    /**
      * The default constructor.
      */
    validation_files_reserved();

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

    // See base class for documentation.
    bool check_downloaded(void) const;

    // See base class for documentation.
    bool check_foreign_copyright(void) const;

    // See base class for documentation.
    bool check_branches(void) const;

private:
    /**
      * The exceptiosn instance variable is used to remember a number of
      * identifiers that are not considered to be errors.
      */
    nstring_list exceptions;

    /**
      * The is_a_reserved_word method is used to test an identifier to
      * determine whether or not it is a reserved word.
      *
      * @param name
      *     The identifoer to be checked.
      * @returns
      *     true if a resewrved word, false if an ordinary word
      *     (it could still be a language keyword).
      */
    bool is_a_reserved_word(const nstring &name) const;

    /**
      * The copy constructor.  Do not use.
      */
    validation_files_reserved(const validation_files_reserved &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_reserved &operator=(const validation_files_reserved &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_RESERVED_H
