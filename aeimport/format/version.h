//
//	aegis - project change supervisor
//	Copyright (C) 2001, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef AEIMPORT_FORMAT_VERSION_H
#define AEIMPORT_FORMAT_VERSION_H

#include <common/ac/time.h>

#include <common/str_list.h>

class format_version_ty
{
public:
    /**
      * The destructor.
      * Do not derived from this class, the destructor isn't  virtual.
      */
    ~format_version_ty();

    /**
      * The default constructor.
      */
    format_version_ty();

    /**
      * The validate method is used to check that this data structure is
      * in a valid state.  Intended for debugging; it will abort() if it
      * finds a problem.
      */
    void validate() const;

//private:
    string_ty       *filename_physical;
    string_ty       *filename_logical;
    string_ty       *edit;
    time_t	    when;
    string_ty	    *who;
    string_ty	    *description;
    string_list_ty  tag;
    format_version_ty *before;
    format_version_ty *after;
    struct format_version_list_ty *after_branch;
    int		    dead;

private:
    /**
      * The copy constructor.  Do not use.
      */
    format_version_ty(const format_version_ty &);

    /**
      * The assignment operator.  Do not use.
      */
    format_version_ty &operator=(const format_version_ty &);
};

#endif // AEIMPORT_FORMAT_VERSION_H
