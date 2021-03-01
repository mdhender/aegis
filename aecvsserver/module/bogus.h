//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#ifndef AECVSSERVER_MODULE_BOGUS_H
#define AECVSSERVER_MODULE_BOGUS_H

#include <aecvsserver/module.h>

/**
  * The module_bogus class is used to represent a non-existent module,
  * which reports errors for all actions attempted on it.
  */
class module_bogus:
    public module_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~module_bogus();

    /**
      * The default constructor.
      */
    module_bogus();

    // See base class for documentation.
    bool is_bogus() const;

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
      * The copy constructor.  Do not use.
      */
    module_bogus(const module_bogus &);

    /**
      * The assignment operator.  Do not use.
      */
    module_bogus &operator=(const module_bogus &);
};

#endif // AECVSSERVER_MODULE_BOGUS_H
