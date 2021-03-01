//
// aegis - project change supervisor
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

#ifndef LIBAEGIS_CHANGE_FUNCTOR_VERSIONS_H
#define LIBAEGIS_CHANGE_FUNCTOR_VERSIONS_H

#include <libaegis/change/functor.h>

/**
  * The change_functor_versions class is used to represent a project
  * inventory of change sets by version numbers.
  */
class change_functor_versions:
    public change_functor
{
public:
    typedef aegis_shared_ptr<change_functor_versions> pointer;

    /**
      * The destructor.
      */
    virtual ~change_functor_versions();

    /**
      * The constructor.
      */
    change_functor_versions(nstring_list &versions);

protected:
    // See base class for documentation.
    void operator()(change::pointer cp);

private:
    nstring_list &versions;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_versions();

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_versions(const change_functor_versions &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_versions &operator=(const change_functor_versions &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_VERSIONS_H
