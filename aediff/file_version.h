//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: interface of the file_version class
//

#ifndef AEDIFF_FILE_VERSION_H
#define AEDIFF_FILE_VERSION_H

#pragma interface "file_version"

#include <nstring.h>

/**
  * The file_version class is used to represent the path to a file
  * version, and wether or not is needs to be unlinked by the
  * destructor.
  */
class file_version
{
public:
    /**
      * The destructor.
      */
    virtual ~file_version();

    /**
      * The constructor.
      */
    file_version(const nstring &filename, bool need_to_unlink);

    /**
      * The copy constructor.
      */
    file_version(const file_version &);

    /**
      * The assignment operator.
      */
    file_version &operator=(const file_version &);

    /**
      * The get_path method is used to extract the path of the file
      * containing the file version.
      */
    nstring get_path() const { return filename; }

private:
    /**
      * The filename instance variable is used to remember the absolute
      * path of the file.
      */
    nstring filename;

    /**
      * The need_to_unlink instance variable is used to remember wether
      * or not we need to unlink the file in the destructor.
      */
    bool need_to_unlink;

    /**
      * The default constructor.
      */
    file_version();
};

#endif // AEDIFF_FILE_VERSION_H
