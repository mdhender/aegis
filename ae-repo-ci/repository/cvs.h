//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#ifndef AE_REPO_CHECKIN_REPOSITORY_CVS_H
#define AE_REPO_CHECKIN_REPOSITORY_CVS_H

#include <common/nstring.h>

#include <ae-repo-ci/repository.h>

/**
  * The repository_cvs class is used to represent the workarea and
  * checkin portion of the interface to a CVS repository.
  */
class repository_cvs:
    public repository
{
public:
    /**
      * The destructor.
      */
    virtual ~repository_cvs();

    /**
      * The default constructor.
      */
    repository_cvs();

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
    void commit(const nstring &comment);

private:
    /**
      * The module instance variable is used to remember the name of the
      * CVS module to be checked out and them checked in again.
      */
    nstring module;

    /**
      * The directory instance variable is used to remember the
      * temporary directory containing the CVS work area.
      */
    nstring directory;

    /**
      * The copy constructor.
      */
    repository_cvs(const repository_cvs &);

    /**
      * The assignment operator.
      */
    repository_cvs &operator=(const repository_cvs &);
};

#endif // AE_REPO_CHECKIN_REPOSITORY_CVS_H
