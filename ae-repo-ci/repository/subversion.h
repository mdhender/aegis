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

#ifndef AE_REPO_CI_REPOSITORY_SUBVERSION_H
#define AE_REPO_CI_REPOSITORY_SUBVERSION_H

#include <ae-repo-ci/repository.h>


/**
  * The repository_subversion class is used to represent the workarea
  * and checkin portion of the interface to a subversion repository.
  */
class repository_subversion:
    public repository
{
public:
    /**
      * The destructor.
      */
    virtual ~repository_subversion();

    /**
      * The default constructor.
      */
    repository_subversion();

    // See base class for documentation.
    void checkout(const nstring &module, const nstring &directory);

    // See base class for documentation.
    nstring get_directory() const;

    // See base class for documentation.
    void remove_file(const nstring &file_name);

    // See base class for documentation.
    void add_file(const nstring &filename, const nstring &content_path);

    // See base class for documentation.
    void modify_file(const nstring &filename, const nstring &content_path);

    // See base class for documentation.
    void rename_file(const nstring &old_filename, const nstring &new_file_name,
	const nstring &content_path);

    // See base class for documentation.
    void commit(const nstring &comment);

    // See base class for documentation.
    void change_specific_attributes(change::pointer cp);

    // See base class for documentation.
    void file_attribute(const nstring &filename, const nstring &attribute_name,
        const nstring &attribute_value);

private:
    /**
      * The module instance variable is used to remember the name of the
      * subversion module to be checked out and them checked in again.
      */
    nstring module;

    /**
      * The directory instance variable is used to remember the
      * temporary directory containing the subversion work area.
      */
    nstring directory;

    /**
      * The auth instance variable is used to remember the
      * authentication command line options (if any) to be passed to
      * svn(1) commands.  If it isn't empty, it always starts with a
      * space.
      */
    nstring auth;

    /**
      * The copy constructor.
      */
    repository_subversion(const repository_subversion &);

    /**
      * The assignment operator.
      */
    repository_subversion &operator=(const repository_subversion &);
};

#endif // AE_REPO_CI_REPOSITORY_SUBVERSION_H
