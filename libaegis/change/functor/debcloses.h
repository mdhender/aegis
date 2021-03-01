//
// aegis - project change supervisor
// Copyright (C) 2009, 2010 Peter Miller
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

#ifndef LIBAEGIS_CHANGE_FUNCTOR_DEBCLOSES_H
#define LIBAEGIS_CHANGE_FUNCTOR_DEBCLOSES_H

#include <common/nstring/list.h>
#include <libaegis/change/functor.h>
#include <common/regula_expre.h>

/**
  * The change_functor_debcloses class is used to represent scanning all
  * change sets for strings indicating Debian bugs have been fixed.
  */
class change_functor_debcloses:
    public change_functor
{
public:
    /**
      * The destructor.
      */
    virtual ~change_functor_debcloses();

    /**
      * The default constructor.
      */
    change_functor_debcloses();

    // See base class for documentation.
    void operator()(change::pointer cp);

    nstring_list get_result(void) const;

private:
    regular_expression closes;
    regular_expression bug;
    regular_expression hash;
    nstring_list result;

    void grope(const nstring &caption, const nstring &text);

    /**
      * The copy constructor.  Do not use.
      */
    change_functor_debcloses(const change_functor_debcloses &);

    /**
      * The assignment operator.  Do not use.
      */
    change_functor_debcloses &operator=(const change_functor_debcloses &);
};

#endif // LIBAEGIS_CHANGE_FUNCTOR_DEBCLOSES_H
