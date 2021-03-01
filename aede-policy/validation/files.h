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

#ifndef AEDE_POLICY_VALIDATION_FILES_H
#define AEDE_POLICY_VALIDATION_FILES_H

#include <aede-policy/validation.h>

/**
  * The validation_files class is used to represent an abstract
  * interface to validations which check all of the files in a change
  * set.
  */
class validation_files:
    public validation
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files();

    /**
      * The default constructor.
      */
    validation_files();

protected:
    // See base class for documentation.
    bool run(change::pointer cp);

    /**
      * The is_a_text_file method may be used to determine whether
      * or not the given file is a text file.  This is based on the
      * file's content-type attribute, if present, or the file itself
      * otherwise.
      *
      * @param src
      *     Details of the file in question.
      * @param cp
      *     the containing change set
      * @returns
      *     bool; true if the file is a text file, false if it is a binary file
      */
    static bool is_a_text_file(fstate_src_ty *src, change::pointer cp);

    /**
      * The check method is used to check a single file.
      *
      * @param cp
      *     The change in question.
      * @param src
      *     The file in question.
      * @returns
      *     bool; true if the file is OK, false if it has a problem
      */
    virtual bool check(change::pointer cp, fstate_src_ty *src) = 0;

    /**
      * The check_branches method is used to determine whether or not
      * branches are to be checked by aede-policy.
      *
      * The default implementation returns true.
      *
      * @returns
      *     bool; true if branches should be checked, false if should not be.
      */
    virtual bool check_branches() const;

    /**
      * The check_downloaded method is used to determine whether or not
      * downloaded change sets are to be checked by aede-policy.
      *
      * The default implementation returns true.
      *
      * @returns
      *     bool; true if downloaded change sets should be checked,
      *     false if should not be.
      */
    virtual bool check_downloaded() const;

    /**
      * The check_foreign_copyright method is used to determine whether
      * or not change sets and files with the foreign-copyright=true
      * attribute are to be checked by aede-policy.
      *
      * The default implementation returns true.
      *
      * @returns
      *     bool; true if foreign copyright change sets should be
      *     checked, false if should not be.
      */
    virtual bool check_foreign_copyright() const;

    /**
      * The check_binaries method is used to determine whether or not
      * files which are NOT text (as determined by is_a_text_file) are
      * to be checked by aede-policy.
      *
      * The default implementation returns true.
      *
      * @returns
      *     bool; true if binary files should be checked, false if
      *     should not be.
      */
    virtual bool check_binaries() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    validation_files(const validation_files &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files &operator=(const validation_files &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_H
