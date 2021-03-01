//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#ifndef AECVSSERVER_MODULE_CHANGE_H
#define AECVSSERVER_MODULE_CHANGE_H

#include <libaegis/change.h>
#include <libaegis/user.h>
#include <aecvsserver/module.h>

struct project_ty; // forward
struct string_ty; // forward

/**
  * The module_change class is used to represent a CVS module
  * synthesized from an Aegis change set.
  */
class module_change:
    public module_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~module_change();

    /**
      * The constructor.
      *
      * @param arg
      *     The change being managed.
      */
    module_change(change::pointer arg);

    // See base class for documentation.
    void modified(server_ty *sp, string_ty *file_name, file_info_ty *fip,
	input &contents);

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
    change::pointer cp;
    project_ty *pp;
    user_ty::pointer up;

    /**
      * The default constructor.
      */
    module_change();

    /**
      * The copy constructor.  Do not use.
      */
    module_change(const module_change &);

    /**
      * The assignment operator.  Do not use.
      */
    module_change &operator=(const module_change &);
};

/**
  * The module_change_new function is used to create a new module based
  * on the project name and the change number.  You rarely want to use
  * "new module_change(cp)" directly because this function will allocate
  * suitable bogus modules if there is an error in the project name or
  * change number.
  */
module_ty *module_change_new(string_ty *projname, long change_number);

#endif // AECVSSERVER_MODULE_CHANGE_H
