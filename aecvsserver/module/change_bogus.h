//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef AECVSSERVER_MODULE_CHANGE_BOGUS_H
#define AECVSSERVER_MODULE_CHANGE_BOGUS_H

#include <aecvsserver/module.h>

/**
  * The module_change_bogus class is used to represent a non-existant
  * change.  It returns errors for all operations.
  */
class module_change_bogus:
    public module_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~module_change_bogus();

    /**
      * The constructor.
      *
      * @param pname
      *     The name of the project
      * @param number
      *     The number of the change set.
      */
    module_change_bogus(string_ty *pname, long number);

    // See base class for documentation.
    bool is_bogus() const;

    // See base class for documentation.
    void modified(server_ty *sp, string_ty *file_name, file_info_ty *fip,
	input &ip);

    // See base class for documentation.
    bool checkin(server_ty *sp, string_ty *client_side,
	string_ty *server_side);

    // See base class for documentation.
    bool update(server_ty *sp, string_ty *client_side,
	string_ty *server_side, const options &opt);

    // See base class for documentation.
    bool add(server_ty *sp, string_ty *client_side,
	string_ty *server_side, const options &opt);

    // See base class for documentation.
    bool remove(server_ty *sp, string_ty *client_side,
	string_ty *server_side, const options &opt);

    // See base class for documentation.
    string_ty *calculate_canonical_name() const;

private:
    /**
      * The pname instance variable is used to remember the name of the
      * project the change set doesn't exist within.
      */
    string_ty *pname;

    /**
      * The number instance variable is used to remember the change set
      * number.
      */
    long number;

    /**
      * The groan method is used to report errors to the client.
      * All method in this class report errors.
      */
    void groan(server_ty *sp, const char *caption);

    /**
      * The default constructor.  Do not use.
      */
    module_change_bogus();

    /**
      * The copy constructor.  Do not use.
      */
    module_change_bogus(const module_change_bogus &);

    /**
      * The assignment operator.  Do not use.
      */
    module_change_bogus &operator=(const module_change_bogus &);
};

#endif // AECVSSERVER_MODULE_CHANGE_BOGUS_H
