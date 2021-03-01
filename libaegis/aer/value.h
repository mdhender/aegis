//
//	aegis - project change supervisor
//	Copyright (C) 1994, 1996, 2002-2008 Peter Miller
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

#ifndef AEGIS_AER_VALUE_H
#define AEGIS_AER_VALUE_H

#include <common/ac/shared_ptr.h>
#include <common/ac/stddef.h>


/**
  * The rpt_value abstract base class is used to represent a generic
  * value, constructed during report generation.
  */
class rpt_value
{
public:
    typedef aegis_shared_ptr<rpt_value> pointer;

    /**
      * The destructor.
      */
    virtual ~rpt_value();

protected:
    /**
      * The default constructor.
      */
    rpt_value();

public:
    /**
      * The integerize class method is used to convert a value to
      * integer, if possible.  If not possible, the value will be
      * returned unchanged.
      *
      * @param vp
      *     The value to convert to an integer.
      * @returns
      *     the integer equaivalent, or unchanged
      */
    static rpt_value::pointer integerize(const rpt_value::pointer &vp);

    /**
      * The realize class method is used to convert a value to
      * real, if possible.  If not possible, the value will be
      * returned unchanged.
      *
      * @param vp
      *     The value to convert to an real.
      * @returns
      *     the real equaivalent, or unchanged
      */
    static rpt_value::pointer realize(const rpt_value::pointer &vp);

    /**
      * The arithmetic class method is used to convert a value to an
      * arithmetic type, if possible.  If not possible, the value will
      * be returned unchanged.
      *
      * @param vp
      *     The value to convert to an arithmetic type.
      * @returns
      *     the arithmetic equivalent, or unchanged
      */
    static rpt_value::pointer arithmetic(const rpt_value::pointer &vp);

    /**
      * The undefer class method is used to evaluate a deferred value,
      * if necessary.  If not necessary, the value will be returned
      * unchanged.
      *
      * @param vp
      *     The deferred value to evaluate.
      * @returns
      *     the calculated result, or unchanged
      */
    static rpt_value::pointer undefer(const rpt_value::pointer &vp);

    /**
      * The stringize class method is used to convert a value to a
      * string, if possible.  If not possible, the value will be
      * returned unchanged.
      *
      * @param vp
      *     The value to convert to a string.
      * @returns
      *     the string result, or unchanged
      */
    static rpt_value::pointer stringize(const rpt_value::pointer &vp);

    /**
      * The booleanize class method is used to convert a value to a
      * boolean, if possible.  If not possible, the value will be
      * returned unchanged.
      *
      * @param vp
      *     The value to convert to a boolean.
      * @returns
      *     the boolean result, or unchanged
      */
    static rpt_value::pointer booleanize(const rpt_value::pointer &vp);

    /**
      * The name method is used to obtain the name of the type of the value.
      */
    virtual const char *name() const = 0;

    /**
      * The is_an_error method may be used to determine whether or
      * not an object instance is an error instance.  This happens
      * often enoygh that the code is clear this way than using a
      * dynamic_cast<rpt_value_error *>
      *
      * @returns
      *     true for error instances, false for everything else
      */
    virtual bool is_an_error() const;

    /**
      * The is_a_struct method may be used to determine whether or not
      * the value is a struct (or associative array).  Superficially,
      * this should be simple, but it is complicated by things like
      * deferred project and change states.
      *
      * @returns
      *      true of struct-like values, false for everything else
      */
    virtual bool is_a_struct() const;

    /**
      * The lookup method is used to index an associate array, or locate
      * a member within a struct.
      *
      * @param rhs
      *     The member name, or array index
      * @param lvalue
      *     The result is to be used of an l-value
      */
    virtual rpt_value::pointer lookup(const rpt_value::pointer &rhs,
        bool lvalue) const;

    /**
      * The keys method is used to obtain a list of keys of an
      * associative array.
      */
    virtual rpt_value::pointer keys() const;

    /**
      * The count method is used to count the number of elements of an
      * associative array.
      */
    virtual rpt_value::pointer count() const;

    /**
      * The type_of method is used to obtain the name of the type of
      * the value.  It differs from the name method in that it resolves
      * references and deferred values.
      */
    virtual const char *type_of() const;

protected:
    /**
      * The integerize_or_null method is used to convert a value to an
      * integer, if possible, or return NULL if it is not possible.
      * This NULL is used by the integerize class method to return the
      * unchanged value if no conversion is possible.
      */
    virtual rpt_value::pointer integerize_or_null() const;

    /**
      * The realize_or_null method is used to convert a value to an
      * real, if possible, or return NULL if it is not possible.  This
      * NULL is used by the realize class method to return the unchanged
      * value if no conversion is possible.
      */
    virtual rpt_value::pointer realize_or_null() const;

    /**
      * The arithmetic_or_null method is used to convert a value to an
      * arithmetic type (real or integer), if possible, or return NULL
      * if it is not possible.  This NULL is used by the integerize
      * class method to return the unchanged value if no conversion is
      * possible.
      */
    virtual rpt_value::pointer arithmetic_or_null() const;

    /**
      * The undefer_or_null method is used to evaluate a deferred value,
      * if necessary, or return NULL if it is not possible or not
      * necessary.  This NULL is used by the undefer class method to
      * return the unchanged value if no conversion is possible.
      */
    virtual rpt_value::pointer undefer_or_null() const;

    /**
      * The stringize_or_null method is used to convert a value to a
      * string, if possible, or return NULL if it is not possible.
      * This NULL is used by the stringize class method to return the
      * unchanged value if no conversion is possible.
      */
    virtual rpt_value::pointer stringize_or_null() const;

    /**
      * The booleanize_or_null method is used to convert a value to a
      * boolean, if possible, or return NULL if it is not possible.
      * This NULL is used by the booleanize class method to return the
      * unchanged value if no conversion is possible.
      */
    virtual rpt_value::pointer booleanize_or_null() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_value(const rpt_value &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_value &operator=(const rpt_value &);
};

#endif // AEGIS_AER_VALUE_H
