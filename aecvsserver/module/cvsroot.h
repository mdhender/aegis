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

#ifndef AECVSSERVER_MODULE_CVSROOT_H
#define AECVSSERVER_MODULE_CVSROOT_H

#include <aecvsserver/module.h>

/**
  * The module_cvsroot class is used to represent a synthesized CVSROOT
  * module.
  */
class module_cvsroot:
    public module_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~module_cvsroot();

    /**
      * The default constructor.
      */
    module_cvsroot();

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
    /**
      * The groan method is used to complain about things we don't like
      * the CVS client asking us to do.
      */
    void groan(server_ty *sp, const char *request_name);

    /**
      * The checkout_modules method is used to synthesize a
      * CVSROOT/modules file checkout.
      */
    void checkout_modules(server_ty *sp);

    /**
      * The copy constructor.  Do not use.
      */
    module_cvsroot(const module_cvsroot &);

    /**
      * The assignment operator.  Do not use.
      */
    module_cvsroot &operator=(const module_cvsroot &);
};

#endif // AECVSSERVER_MODULE_CVSROOT_H
