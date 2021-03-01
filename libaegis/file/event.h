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

#ifndef LIBAEGIS_FILE_EVENT_H
#define LIBAEGIS_FILE_EVENT_H

#include <common/ac/time.h>

#include <libaegis/change.h>

struct fstate_src_ty; // forward

/**
  * The file_event class is used to represent a change set as an event
  * that happened to a file.
  */
class file_event
{
public:
    /**
      * The destructor.
      */
    virtual ~file_event();

    /**
      * The default constructor.
      */
    file_event();

    /**
      * The constructor.
      */
    file_event(time_t when, change::pointer cp, fstate_src_ty *src);

    /**
      * The copy constructor.
      */
    file_event(const file_event &);

    /**
      * The assignment operator.
      */
    file_event &operator=(const file_event &);

    time_t get_when() const { return when; }
    change::pointer get_change() const { return cp; }
    fstate_src_ty *get_src() const { return src; }

private:
    time_t when;
    change::pointer cp;
    fstate_src_ty *src;
};

#endif // LIBAEGIS_FILE_EVENT_H
