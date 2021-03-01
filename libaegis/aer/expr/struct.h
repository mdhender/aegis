//
//	aegis - project change supervisor
//	Copyright (C) 1994, 2002, 2005-2008 Peter Miller.
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

#ifndef AEGIS_AER_EXPR_STRUCT_H
#define AEGIS_AER_EXPR_STRUCT_H

#include <libaegis/aer/expr.h>

class rpt_value_struct; // forward

/**
  * The rpt_expr_struct class is used to represent the state of build a
  * struct expression node.
  */
class rpt_expr_struct:
    public rpt_expr
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_expr_struct();

private:
    /**
      * The default constructor.
      * It's private on purpisem use the "create" class method instead.
      */
    rpt_expr_struct();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static rpt_expr::pointer create();

    /**
      * @note
      *     This method is only public so that rpt_expr_struct_assign
      *     can access it.  No other object shall use this method.
      */
    static rpt_value_struct *symtab_query();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    rpt_value::pointer evaluate() const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_expr_struct(const rpt_expr_struct &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_expr_struct &operator=(const rpt_expr_struct &);
};

struct symtab_ty *rpt_expr_struct__symtab_query(void);

#endif // AEGIS_AER_EXPR_STRUCT_H
