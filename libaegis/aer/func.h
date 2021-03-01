//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002-2008 Peter Miller.
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

#ifndef AEGIS_AER_FUNC_H
#define AEGIS_AER_FUNC_H

#include <common/ac/shared_ptr.h>
#include <common/ac/stddef.h>

#include <common/symtab/template.h>
#include <libaegis/aer/expr.h>
#include <libaegis/aer/value.h>


/**
  * The rpt_func abstract base class is used to represent a generic
  * function callable from within the report generator.
  */
class rpt_func
{
public:
    typedef aegis_shared_ptr<rpt_func> pointer;

    /**
      * The destructor.
      */
    virtual ~rpt_func();

protected:
    /**
      * The default constructor.
      */
    rpt_func();

public:
    /**
      * The name method may be used to obtain the name of the function.
      */
    virtual const char *name() const = 0;

    /**
      * The optimizable method may be used to determine whether or not
      * this function can be optimized.
      */
    virtual bool optimizable() const = 0;

    /**
      * The verify method may be used to determine whether or not the
      * arguments presented are valid for this function.
      *
      * @param ep
      *     The list of arguments
      */
    virtual bool verify(const rpt_expr::pointer &ep) const = 0;

    /**
      * The run method is used to evaluate the value of this function.
      *
      * @param ep
      * @param argc
      * @param argv
      */
    virtual rpt_value::pointer run(const rpt_expr::pointer &ep, size_t argc,
        rpt_value::pointer *argv) const = 0;

    /**
      * The init class method is used to initialize the given symnbl
      * table with rpt_func instances.
      */
    static void init(symtab<rpt_value::pointer> &);
};

#endif // AEGIS_AER_FUNC_H
