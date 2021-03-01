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

#ifndef LIBAEGIS_FILE_EVENT_LIST_H
#define LIBAEGIS_FILE_EVENT_LIST_H

#include <common/ac/stddef.h>

class file_event; // forward

/**
  * The file_event_list class is used to represent an ordered list of
  * file_event instances.
  */
class file_event_list
{
public:
    typedef aegis_shared_ptr<file_event_list> pointer;

    /**
      * The destructor.
      */
    virtual ~file_event_list();

private:
    /**
      * The default constructor.  It is private on purpose, use a
      * "create" class method instead.
      */
    file_event_list();

    /**
      * The copy constructor.  It is private on purpose, use a "create"
      * class method instead.
      */
    file_event_list(const file_event_list &);

    /**
      * The assignment operator.  It is private on purpose, use a
      * "create" class method instead.
      */
    file_event_list &operator=(const file_event_list &);

public:
    static pointer create();

    static pointer create(const pointer &arg);

    size_t size() const { return length; }
    void clear();
    void push_back(const file_event_list &arg);
    void push_back(file_event *arg);

    file_event *get(size_t n) const { return item[n]; }
    file_event *operator[](size_t n) const { return get(n); }
    bool empty() const { return (length == 0); }
    file_event *back() const { return item[length - 1]; }

private:
    size_t length;
    size_t maximum;
    file_event **item;
};

#endif // LIBAEGIS_FILE_EVENT_LIST_H
