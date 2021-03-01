//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002, 2005-2008 Peter Miller
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

#ifndef AEGIS_AER_VALUE_STRUCT_H
#define AEGIS_AER_VALUE_STRUCT_H

#include <common/symtab/template.h>
#include <libaegis/aer/value.h>

/**
  * The rpt_value_struct class is used to represent a vales consisting
  * of zero or more name-value pairs.
  */
class rpt_value_struct:
    public rpt_value
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_value_struct();

    /**
      * The constructor.  Use the "create" class method instead.
      *
      * @note
      *     This method is only public so that rpt_expr_struct::evaluate
      *     can use it.  No other objects shall use this constructor.
      */
    rpt_value_struct();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static rpt_value::pointer create();

    /**
      * The assign method is used to set a struct member by name.
      *
      * @param name
      *     The name of the struct member.
      * @param value
      *     The value of the struct member.
      */
    void assign(const nstring &name, const rpt_value::pointer &value);

    /**
      * The lookup method is used to obtain the vale of the named struct
      * member.
      *
      * @param name
      *     The name of the member to look for.
      * @returns
      *     the value of the member, or nul if not present.
      */
    rpt_value::pointer lookup(const char *name) const;

    /**
      * The lookup method is used to obtain the vale of the named struct
      * member.
      *
      * @param name
      *     The name of the member to look for.
      * @returns
      *     the value of the member, or nul if not present.
      */
    rpt_value::pointer lookup(const nstring &name) const;

    /**
      * The lookup method is used to obtain the vale of the named struct
      * member.
      *
      * @param name
      *     The name of the member to look for.
      * @returns
      *     the value of the member, or nul if not present.
      */
    rpt_value::pointer lookup(string_ty *name) const;

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool is_a_struct() const;

    // See base class for documentation.
    rpt_value::pointer undefer() const;

    // See base class for documentation.
    rpt_value::pointer lookup(const rpt_value::pointer &rhs, bool lvalue) const;

    // See base class for documentation.
    rpt_value::pointer keys() const;

    // See base class for documentation.
    rpt_value::pointer count() const;

private:
    /**
      * The members instance variable is used ro remember the value of
      * each member of the struct.
      *
      * It is mutable in that there are times when lookup() will create
      * an entry with a nul value, as if it contains an infinite set of
      * nul values.
      */
    mutable symtab<rpt_value::pointer> members;

    /**
      * The copy constructor.  Do not use.
      */
    rpt_value_struct(const rpt_value_struct &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value_struct &operator=(const rpt_value_struct &);
};

#endif // AEGIS_AER_VALUE_STRUCT_H
