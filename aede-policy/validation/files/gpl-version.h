//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 3 of
//      the License, or (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef AEDE_POLICY_VALIDATION_FILES_GPL_VERSION_H
#define AEDE_POLICY_VALIDATION_FILES_GPL_VERSION_H

#include <aede-policy/validation/files.h>

/**
  * The validation_files_gpl_version class is used to represent a check
  * of the GPL version number in the file header comments.
  */
class validation_files_gpl_version:
    public validation_files
{
public:
    /**
      * The destructor.
      */
    virtual ~validation_files_gpl_version();

    /**
      * The default constructor.
      */
    validation_files_gpl_version(int version = 3);

protected:
    // See base class for documentation
    bool check(change::pointer cp, fstate_src_ty *src);

private:
    /**
      * The version instance variable is used to remember the desired
      * version number of the GNU GPL cited in file header comments.
      */
    int version;

    /**
      * The match function is used to match a buffer full of text against an
      * address pattern.
      *
      * @param text
      *     The text to be matched to the pattern.
      * @param nbytes
      *     The length of the text to be matched by the pattern.
      * @param pattern
      *     The pattern to be matched to.  NUL terminated C string.  All
      *     isalpha(c) are litterals.  The star (*) character marks where
      *     where the version number occurs in the pattern.  Runs
      *     of !isalnum(c) wild match runs of !isalnum(c) in the text.
      *
      * @returns
      *     0 for no match, or line number for a match
      */
    int match(const char *text, long nbytes, const char *pattern) const;

    /**
      * The copy constructor.  Do not use.
      */
    validation_files_gpl_version(const validation_files_gpl_version &);

    /**
      * The assignment operator.  Do not use.
      */
    validation_files_gpl_version &operator=(
        const validation_files_gpl_version &);
};

#endif // AEDE_POLICY_VALIDATION_FILES_GPL_VERSION_H
