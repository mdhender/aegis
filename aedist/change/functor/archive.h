//
// aegis - project change supervisor
// Copyright (C) 2007 Walter Franzini
// Copyright (C) 2008 Peter Miller
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

#ifndef AEDIST_CHANGE_FUNCTOR_ARCHIVE_H
#define AEDIST_CHANGE_FUNCTOR_ARCHIVE_H

#include <libaegis/change/functor.h>


/**
  * The change_functor_archive class is used to populate the current
  * directory with files generated with aedist(1) -send.
  * For each completed change with an UUID in the project a file is
  * generated.  The file is the output of the 'aedist -send' command
  * and is named after the UUID of the change.
  * To detect corruptions, for each file the fingerprint is calculated
  * and stored in another file.
  */
class change_functor_archive:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_archive();

    /**
      * The constructor.
      *
      * @param incbr
      *     whether or not to recurse down brabches
      * @param pp
      *     The project in question.
      * @param suffix
      *     The suffix to append to aedist(1) generated files.
      * @param fingerprint_suffix
      *     The suffix to append to files containing the fingerprint
      *     of aedist(1) generated archives.
      * @param include_change
      *     The list of version patterns used to filter changes from
      *     the inventory.
      * @param exclude_change
      *     The list of version patterns used to filter out changes
      *     from the inventory.
      */
    change_functor_archive(bool incbr, project_ty *pp,
        const char *suffix, const char *fingerprint_suffix,
        const nstring_list&, const nstring_list&);

    /**
      * The operator() is run for each completed change with an UUID.
      */
    void operator()(change::pointer );

private:

    /**
      * The exclude_change instance variable is used to remember the
      * list of patterns used to filter out changes.  The patterns
      * match against the change's version.
      */
    nstring_list exclude_change;

    /**
      * The fingerprint_suffix instance variable is used to remember
      * the extension to be appended to the fingerprin file.
      */
    nstring fingerprint_suffix;

    /**
      * The include_change instance variable is used to remember the
      * list of patterns used to filter changes.  The patterns
      * match against the change's version.
      */
    nstring_list include_change;

    /**
      * The pp instance variable is used to remember the project being listed.
      */
    project_ty *pp;

    /**
      * The suffix instance variable is used to remember the extension
      * to be appendedn to the aedist(1) generated archive file.
      */
    nstring suffix;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_archive();

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_archive(const change_functor_archive &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_archive &operator=(const change_functor_archive &);
};

#endif // AEDIST_CHANGE_FUNCTOR_ARCHIVE_H
