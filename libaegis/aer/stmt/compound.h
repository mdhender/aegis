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

#ifndef AEGIS_AER_STMT_COMPOUND_H
#define AEGIS_AER_STMT_COMPOUND_H

#include <libaegis/aer/stmt.h>

/**
  * The rpt_stmt_compound class is sued to represent a coumpound
  * statement node of the syntax tree.
  */
class rpt_stmt_compound:
    public rpt_stmt
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_stmt_compound();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the "create" class method instead.
      */
    rpt_stmt_compound();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create();

protected:
    // See base class for documentation.
    void run(rpt_stmt_result_ty *) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_stmt_compound(const rpt_stmt_compound &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_stmt_compound &operator=(const rpt_stmt_compound &);
};

#endif // AEGIS_AER_STMT_COMPOUND_H
