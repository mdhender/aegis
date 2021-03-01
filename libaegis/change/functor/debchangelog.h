//
// aegis - project change supervisor
// Copyright (C) 2008, 2009 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_FUNCTOR_DEBCHANGELOG_H
#define LIBAEGIS_CHANGE_FUNCTOR_DEBCHANGELOG_H

#include <libaegis/change/functor.h>
#include <libaegis/col.h>

/**
  * The change_functor_debchangelog class is used to represent writing a
  * debian change log for entry for a change.
  */
class change_functor_debchangelog:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_debchangelog();

    /**
      * The constructor.
      */
    change_functor_debchangelog(col::pointer colp);

    // See base class for documentation.
    void operator()(change::pointer cp);

    /**
      * The print method is used to print a changelog entry for the
      * given change set.  Unlink the operator() method, it doesn't
      * censor branches.
      */
    void print(change::pointer cp);

    /**
      * The print method is used to print a changelog entry for the
      * given change set.  Unlink the operator() method, it doesn't
      * censor branches.
      *
      * @param cp
      *     The change set to be printed
      * @param extra
      *     Extra change-log lines to be printed.
      */
    void print(change::pointer cp, const nstring_list &extra);

private:
    col::pointer colp;
    output::pointer heading;
    output::pointer star;
    output::pointer description;
    output::pointer signature;

    /**
      * The default constructor.  Do not use.
      */
    change_functor_debchangelog();

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_debchangelog(const change_functor_debchangelog &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_debchangelog &operator=(const change_functor_debchangelog &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_DEBCHANGELOG_H
