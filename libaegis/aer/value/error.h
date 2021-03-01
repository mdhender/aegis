//
//	aegis - project change supervisor
//	Copyright (C) 1994-1996, 2002, 2005-2008 Peter Miller
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

#ifndef AEGIS_AER_VALUE_ERROR_H
#define AEGIS_AER_VALUE_ERROR_H

#include <common/nstring.h>
#include <libaegis/aer/pos.h>
#include <libaegis/aer/value.h>


/**
  * The rpt_value_error class is used to represent a calculation error,
  * including a human readable textual representation.
  */
class rpt_value_error:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_error();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      *
      * @param where
      *     Where the error occurred.
      * @param what
      *     What went wrong.
      */
    rpt_value_error(const rpt_position::pointer &where, const nstring &what);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param what
      *     What went wrong.
      */
    static rpt_value::pointer create(string_ty *what) DEPRECATED;

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param what
      *     What went wrong.
      */
    static rpt_value::pointer create(const nstring &what);

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param where
      *     Where the error occurred.
      * @param what
      *     What went wrong.
      */
    static rpt_value::pointer create(const rpt_position::pointer &where,
        string_ty *what) DEPRECATED;

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param where
      *     Where the error occurred.
      * @param what
      *     What went wrong.
      */
    static rpt_value::pointer create(const rpt_position::pointer &where,
        const nstring &what);

    /**
      * The query method may be used to obtain the human readable
      * representation of the error.
      */
    nstring query() const;

    /**
      * The setpos method may be used to update the recorded position of
      * the error.
      */
    void setpos(const rpt_position::pointer &where);

    /**
      * The print method may be used to print the error message,
      * including the position if one has been provided.
      */
    void print() const;

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool is_an_error() const;

private:
    /**
      * The where instance variable is used to remember the source code
      * location of the error.
      */
    rpt_position::pointer where;

    /**
      * The what instance variable is used to remember the human
      * readable text of the error.
      */
    nstring what;

    /**
      * The default constructor.  Do not use.
      */
    rpt_value_error();

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_error(const rpt_value_error &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_error &operator=(const rpt_value_error &);
};

#endif // AEGIS_AER_VALUE_ERROR_H
