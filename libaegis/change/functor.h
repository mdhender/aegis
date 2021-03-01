//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_FUNCTOR_H
#define LIBAEGIS_CHANGE_FUNCTOR_H

#include <common/ac/time.h>

#include <libaegis/change.h>

/**
  * The change_functor class is used to represent an abstract base class
  * used to present object which <i>look like</i> callable functions
  * which take a singe change::pointer  argument.
  *
  * Typically, this is used with the ... class to walk the changes of
  * branch trees.
  */
class change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor();

    /**
      * The operator() method is used to call the functor.
      *
      * \param cp
      *     The change to be operated upon.
      */
    virtual void operator()(change::pointer cp) = 0;

    /**
      * The include_branches method is sued to determine whether or not
      * the functor should also be invoked for branches, when performing
      * a recursive descent.
      */
    bool include_branches() const { return include_branches_flag; }

    /**
      * The all_changes method is used to determine whether or not the
      * functor should be invoked for incomplete changes as well as
      * completed changes and branches.
      */
    bool all_changes() const { return all_changes_flag; }

    /**
      * The earliest method is used to determine the earliest time of
      * interest to the functor.
      *
      * @returns
      *      time_t; the default implementation returns zero.
      */
    virtual time_t earliest();

    /**
      * The lastest method is used to determine the latest time of
      * interest to the functor.
      *
      * @returns
      *      time_t; the default implementation returns "now".
      */
    virtual time_t latest();

    /**
      * The recurse_branches method may be used to determine whether to
      * recurse into nested branches or not.
      *
      * @returns
      *     bool; true if recursion is desired, false if not.
      *     The default implementation returns true.
      */
    virtual bool recurse_branches();

protected:
    /**
      * The constructor.
      * May ony be called by a derived class.
      */
    change_functor(bool include_branches, bool all_changes = false);

    /**
      * The constructor.
      * May ony be called by a derived class.
      */
    change_functor(const change_functor &);

    /**
      * The assignment operator.
      * May ony be called by a derived class.
      */
    change_functor &operator=(const change_functor &);

private:
    bool include_branches_flag;
    bool all_changes_flag;

    /**
      * The default constructor.  Do not use.
      */
    change_functor();
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_H
